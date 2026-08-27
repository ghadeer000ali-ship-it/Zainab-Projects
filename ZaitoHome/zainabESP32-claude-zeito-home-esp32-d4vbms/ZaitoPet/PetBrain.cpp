/*
 * PetBrain.cpp
 * ------------
 * Behavior/animation state machine. See PetBrain.h for the field-level
 * design. This file is organized top-to-bottom as:
 *   1. update() and its per-tick step functions
 *   2. one function per trigger/animation
 *   3. small helpers (random interval, easing, expression picker)
 *
 * Every timer is "next fire time in millis()", rolled to a new random
 * value from PetConfig's [MIN,MAX] range each time it fires - this is
 * what keeps Zaito from ever looking like it's on a fixed, robotic
 * cycle (see PetConfig.h for the actual ranges).
 */

#include "PetBrain.h"
#include "PetConfig.h"
#include <math.h>

using namespace PetConfig;

PetBrain::PetBrain() {}

void PetBrain::begin() {
  // Cheap, portable entropy source - we only need "doesn't repeat the
  // same animation pattern every power-up", not cryptographic quality.
  randomSeed(micros());

  unsigned long now = millis();
  _nextBlinkAt = now + randomInterval(BLINK_INTERVAL_MIN, BLINK_INTERVAL_MAX);
  _nextLookAt = now + randomInterval(LOOK_INTERVAL_MIN, LOOK_INTERVAL_MAX);
  _nextYawnAt = now + randomInterval(YAWN_INTERVAL_MIN, YAWN_INTERVAL_MAX);
  _nextExpressionChangeAt = now + randomInterval(EXPRESSION_INTERVAL_MIN, EXPRESSION_INTERVAL_MAX);
  _nextSleepAt = now + randomInterval(SLEEP_INTERVAL_MIN, SLEEP_INTERVAL_MAX);
  _nextSweatAt = now + randomInterval(SWEAT_INTERVAL_MIN, SWEAT_INTERVAL_MAX);

  _face = FaceState();
}

// ===========================================================
// update() - called every loop(). No delay() anywhere below: every
// animation advances purely as a function of millis(), so update()
// always returns immediately.
// ===========================================================
void PetBrain::update() {
  unsigned long now = millis();

  updateScheduledTimers(now);   // may start new transient anims / change mood / sleep / wake
  updateActiveAnimation(now);   // progresses blink / wink / look-around / yawn
  updateSleepTransition(now);   // eases eyelids shut/open around sleep enter/exit
  updateReactionBursts(now);    // progresses the jump / shake bursts
  updateBreathing(now);         // idle breathing sway (skipped while jumping)
  updateOverlay(now);           // expires the heart/sweat/anger/sparkle burst
  composeFaceState();           // final assembly for PetFace
}

// ===========================================================
// Step 1: fire any timers that have come due
// ===========================================================
void PetBrain::updateScheduledTimers(unsigned long now) {
  // A forced expression (see forceExpression()) expires on its own.
  if (_expressionForced && now >= _forcedUntilMs) {
    _expressionForced = false;
    _nextExpressionChangeAt = now; // pick a fresh mood right away
  }

  if (_petState == PetState::AWAKE) {
    // Discrete eye/mouth animations are mutually exclusive - only fire
    // a new one once the previous one has finished.
    if (_activeAnim == TransientAnim::NONE) {
      if (now >= _nextBlinkAt) {
        triggerBlink(now);
      } else if (now >= _nextLookAt) {
        triggerLookAround(now);
      } else if (now >= _nextYawnAt) {
        triggerYawn(now);
      }
    }

    if (!_expressionForced && now >= _nextExpressionChangeAt) {
      changeExpression(pickRandomExpression(), now);
    }

    if (now >= _nextSweatAt) {
      triggerSweatFlash(now);
    }

    if (now >= _nextSleepAt) {
      enterSleep(now);
    }
  } else { // SLEEPING
    if (now >= _nextWakeAt) {
      wakeUp(now);
    }
  }
}

// ===========================================================
// Step 2: progress whichever discrete animation is currently active
// ===========================================================
void PetBrain::updateActiveAnimation(unsigned long now) {
  if (_activeAnim == TransientAnim::NONE) return;

  float t = (float)(now - _animStartMs) / (float)_animDurationMs;
  if (t > 1.0f) t = 1.0f;
  bool finished = (t >= 1.0f);

  switch (_activeAnim) {
    case TransientAnim::BLINK: {
      float openness = 1.0f - easeEnvelope(t);
      _face.eyeOpennessL = openness;
      _face.eyeOpennessR = openness;
      break;
    }
    case TransientAnim::WINK: {
      float openness = 1.0f - easeEnvelope(t);
      if (_winkEye < 0) _face.eyeOpennessL = openness;
      else _face.eyeOpennessR = openness;
      break;
    }
    case TransientAnim::LOOK_AROUND: {
      float env = easeEnvelope(t);
      _face.gazeX = (int8_t)lroundf(_lookTargetX * env);
      _face.gazeY = (int8_t)lroundf(_lookTargetY * env);
      break;
    }
    case TransientAnim::YAWN: {
      // Eyes droop to ~40% open (not fully shut) while the mouth opens
      // wide - PetFace reads FaceState::isYawning to swap in the yawn
      // mouth; here we only own the eye-openness portion.
      float openness = 1.0f - easeEnvelope(t) * 0.6f;
      _face.eyeOpennessL = openness;
      _face.eyeOpennessR = openness;
      break;
    }
    default: break;
  }

  if (finished) {
    _face.eyeOpennessL = 1.0f;
    _face.eyeOpennessR = 1.0f;
    _face.gazeX = 0;
    _face.gazeY = 0;
    _activeAnim = TransientAnim::NONE;
  }
}

// ===========================================================
// Step 3: ease eyelids around sleep enter/exit
// ===========================================================
void PetBrain::updateSleepTransition(unsigned long now) {
  if (!_sleepTransitionActive) return;

  float t = (float)(now - _sleepTransitionStartMs) / (float)SLEEP_EYE_TRANSITION;
  if (t > 1.0f) t = 1.0f;

  float val = _sleepTransitionFrom + (_sleepTransitionTo - _sleepTransitionFrom) * t;
  _face.eyeOpennessL = val;
  _face.eyeOpennessR = val;

  if (t >= 1.0f) _sleepTransitionActive = false;
}

// ===========================================================
// Step 4: one-shot reaction bursts (jump on happy, shake on laughing)
// ===========================================================
void PetBrain::updateReactionBursts(unsigned long now) {
  if (_jumpActive) {
    float t = (float)(now - _jumpStartMs) / (float)JUMP_DURATION;
    if (t >= 1.0f) {
      _jumpActive = false;
      _face.bodyOffsetY = 0; // updateBreathing() will take over again next step
    } else {
      float arc = 4.0f * t * (1.0f - t); // 0 -> 1 -> 0 parabola
      _face.bodyOffsetY = -(int8_t)lroundf(JUMP_HEIGHT * arc);
    }
  }

  if (_shakeActive) {
    float t = (float)(now - _shakeStartMs) / (float)SHAKE_DURATION;
    if (t >= 1.0f) {
      _shakeActive = false;
      _face.bodyOffsetX = 0;
    } else {
      float wobble = sinf(t * 2.0f * (float)PI * 4.0f) * (1.0f - t); // decaying wobble
      _face.bodyOffsetX = (int8_t)lroundf(SHAKE_AMPLITUDE * wobble);
    }
  } else {
    _face.bodyOffsetX = 0;
  }
}

// ===========================================================
// Step 5: continuous idle breathing sway
// ===========================================================
void PetBrain::updateBreathing(unsigned long now) {
  if (_jumpActive) return; // jump owns bodyOffsetY while it's playing

  unsigned long period = (_petState == PetState::SLEEPING) ? BREATH_PERIOD * 2 : BREATH_PERIOD;
  float amplitude = (_petState == PetState::SLEEPING) ? BREATH_AMPLITUDE * 0.5f : BREATH_AMPLITUDE;
  float phase = (float)(now % period) / (float)period;
  _face.bodyOffsetY = (int8_t)lroundf(sinf(phase * 2.0f * (float)PI) * amplitude);
}

// ===========================================================
// Step 6: expire the current overlay icon burst
// ===========================================================
void PetBrain::updateOverlay(unsigned long now) {
  if (_overlay != OverlayType::NONE && now - _overlayStartMs >= _overlayDurationMs) {
    _overlay = OverlayType::NONE;
  }
}

// ===========================================================
// Final assembly
// ===========================================================
void PetBrain::composeFaceState() {
  _face.expression = _currentExpression;
  _face.isAsleep = (_petState == PetState::SLEEPING);
  _face.isYawning = (_activeAnim == TransientAnim::YAWN);
  _face.overlay = (_petState == PetState::SLEEPING) ? OverlayType::ZZZ : _overlay;

  // Resting gaze: only meaningful when no discrete look-around
  // animation is currently driving gazeX/gazeY itself.
  if (_activeAnim != TransientAnim::LOOK_AROUND) {
    if (_currentExpression == Expression::THINKING && _petState == PetState::AWAKE) {
      // Tiny, purely cosmetic drift so the "thinking" gaze doesn't look
      // frozen in place - the DECISION to hold a thinking gaze is made
      // here; the sub-pixel wobble itself carries no behavioral meaning.
      float drift = sinf(millis() / 1500.0f);
      _face.gazeX = GAZE_RANGE_X - 1;
      _face.gazeY = (int8_t)(-(GAZE_RANGE_Y_UP - 1) + lroundf(drift));
    } else {
      _face.gazeX = 0;
      _face.gazeY = 0;
    }
  }

  // Resting eye openness: only when nothing is actively holding it
  // elsewhere (a blink/wink/yawn in progress, or a sleep transition).
  if (_activeAnim == TransientAnim::NONE && !_sleepTransitionActive &&
      _petState == PetState::AWAKE) {
    _face.eyeOpennessL = 1.0f;
    _face.eyeOpennessR = 1.0f;
  }
}

// ===========================================================
// One function per animation/trigger
// ===========================================================
void PetBrain::triggerBlink(unsigned long now) {
  // Occasionally wink instead of blinking - only while in an upbeat
  // mood, so it reads as an intentional, cheeky gesture rather than a
  // random tic.
  bool canWink = (_currentExpression == Expression::HAPPY ||
                   _currentExpression == Expression::EXCITED ||
                   _currentExpression == Expression::NORMAL);
  if (canWink && random(100) < 15) {
    triggerWink(now);
    return;
  }
  _activeAnim = TransientAnim::BLINK;
  _animStartMs = now;
  _animDurationMs = BLINK_DURATION;
  _nextBlinkAt = now + randomInterval(BLINK_INTERVAL_MIN, BLINK_INTERVAL_MAX);
}

void PetBrain::triggerWink(unsigned long now) {
  _activeAnim = TransientAnim::WINK;
  _animStartMs = now;
  _animDurationMs = WINK_DURATION;
  _winkEye = (random(2) == 0) ? -1 : 1;
  _nextBlinkAt = now + randomInterval(BLINK_INTERVAL_MIN, BLINK_INTERVAL_MAX);
}

void PetBrain::triggerLookAround(unsigned long now) {
  _activeAnim = TransientAnim::LOOK_AROUND;
  _animStartMs = now;
  _animDurationMs = LOOK_DURATION;

  static const int8_t targetsX[] = { -GAZE_RANGE_X, GAZE_RANGE_X, 0, -GAZE_RANGE_X, GAZE_RANGE_X };
  static const int8_t targetsY[] = { 0, 0, -GAZE_RANGE_Y_UP, -GAZE_RANGE_Y_UP / 2, -GAZE_RANGE_Y_UP / 2 };
  uint8_t pick = random(5);
  _lookTargetX = targetsX[pick];
  _lookTargetY = targetsY[pick];

  _nextLookAt = now + randomInterval(LOOK_INTERVAL_MIN, LOOK_INTERVAL_MAX);
}

void PetBrain::triggerYawn(unsigned long now) {
  _activeAnim = TransientAnim::YAWN;
  _animStartMs = now;
  _animDurationMs = YAWN_DURATION;
  _nextYawnAt = now + randomInterval(YAWN_INTERVAL_MIN, YAWN_INTERVAL_MAX);

  // A yawn is a good sign a nap is coming soon - nudge (never delay)
  // the sleep timer closer if it was going to be a long wait.
  unsigned long soon = now + randomInterval(4000, 9000);
  if (_nextSleepAt > soon) _nextSleepAt = soon;
}

void PetBrain::triggerJump(unsigned long now) {
  _jumpActive = true;
  _jumpStartMs = now;
}

void PetBrain::triggerShake(unsigned long now) {
  _shakeActive = true;
  _shakeStartMs = now;
}

void PetBrain::triggerSweatFlash(unsigned long now) {
  _nextSweatAt = now + randomInterval(SWEAT_INTERVAL_MIN, SWEAT_INTERVAL_MAX);
  // Don't interrupt whatever overlay (if any) is already showing.
  if (_overlay == OverlayType::NONE) {
    triggerOverlay(OverlayType::SWEAT, now, SWEAT_DURATION);
  }
}

void PetBrain::triggerOverlay(OverlayType type, unsigned long now, unsigned long durationMs) {
  _overlay = type;
  _overlayStartMs = now;
  _overlayDurationMs = durationMs;
}

void PetBrain::enterSleep(unsigned long now) {
  _petState = PetState::SLEEPING;
  _activeAnim = TransientAnim::NONE;

  _sleepTransitionActive = true;
  _sleepTransitionStartMs = now;
  _sleepTransitionFrom = 1.0f;
  _sleepTransitionTo = 0.05f;

  _nextWakeAt = now + randomInterval(SLEEP_DURATION_MIN, SLEEP_DURATION_MAX);
}

void PetBrain::wakeUp(unsigned long now) {
  _petState = PetState::AWAKE;
  _currentExpression = Expression::NORMAL;

  _sleepTransitionActive = true;
  _sleepTransitionStartMs = now;
  _sleepTransitionFrom = 0.05f;
  _sleepTransitionTo = 1.0f;

  // Give Zaito a moment fully awake before the random schedulers can
  // fire again, so it doesn't blink/change mood/nod off mid-yawn-stretch.
  _nextBlinkAt = now + randomInterval(BLINK_INTERVAL_MIN, BLINK_INTERVAL_MAX);
  _nextLookAt = now + randomInterval(LOOK_INTERVAL_MIN, LOOK_INTERVAL_MAX);
  _nextYawnAt = now + randomInterval(YAWN_INTERVAL_MIN, YAWN_INTERVAL_MAX);
  _nextExpressionChangeAt = now + randomInterval(EXPRESSION_INTERVAL_MIN, EXPRESSION_INTERVAL_MAX);
  _nextSleepAt = now + randomInterval(SLEEP_INTERVAL_MIN, SLEEP_INTERVAL_MAX);
}

void PetBrain::changeExpression(Expression next, unsigned long now) {
  _previousExpression = _currentExpression;
  _currentExpression = next;
  _nextExpressionChangeAt = now + randomInterval(EXPRESSION_INTERVAL_MIN, EXPRESSION_INTERVAL_MAX);

  // Reaction bursts tied to entering specific moods.
  switch (next) {
    case Expression::HAPPY:
    case Expression::EXCITED:
      triggerJump(now);
      triggerOverlay(OverlayType::HEART, now, OVERLAY_BURST_DURATION);
      break;
    case Expression::LAUGHING:
      triggerShake(now);
      break;
    case Expression::ANGRY:
      triggerOverlay(OverlayType::ANGER, now, OVERLAY_BURST_DURATION);
      break;
    case Expression::SURPRISED:
      triggerOverlay(OverlayType::SPARKLE, now, OVERLAY_BURST_DURATION);
      break;
    default:
      break;
  }
}

// ===========================================================
// Public hooks reserved for future input sources
// ===========================================================
void PetBrain::notifyInteraction() {
  unsigned long now = millis();
  if (_petState == PetState::SLEEPING) {
    wakeUp(now);
    return;
  }
  changeExpression(Expression::HAPPY, now);
}

void PetBrain::forceExpression(Expression e, unsigned long durationMs) {
  unsigned long now = millis();
  _expressionForced = true;
  _forcedUntilMs = now + durationMs;
  changeExpression(e, now);
}

// ===========================================================
// Small helpers
// ===========================================================
Expression PetBrain::pickRandomExpression() {
  Expression candidate;
  do {
    candidate = (Expression)random((long)Expression::COUNT);
  } while (candidate == _currentExpression);
  return candidate;
}

unsigned long PetBrain::randomInterval(unsigned long minMs, unsigned long maxMs) {
  return (unsigned long)random((long)minMs, (long)maxMs + 1);
}

float PetBrain::easeEnvelope(float t) {
  if (t < 0.0f) t = 0.0f;
  if (t > 1.0f) t = 1.0f;
  return sinf((float)PI * t);
}
