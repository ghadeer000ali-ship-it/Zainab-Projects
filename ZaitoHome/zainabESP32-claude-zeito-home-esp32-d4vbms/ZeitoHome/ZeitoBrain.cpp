/*
 * ZeitoBrain.cpp
 * --------------
 * See ZeitoBrain.h for the field-level design. Organized top-to-bottom
 * as:
 *   1. update() and its per-tick step functions
 *   2. one function per activity's per-tick behavior
 *   3. public hooks (hand detected / menu commands)
 *   4. small helpers (walking, random interval, dream icon picker)
 */

#include "ZeitoBrain.h"
#include "HomeConfig.h"

using namespace HomeConfig;

ZeitoBrain::ZeitoBrain() {}

void ZeitoBrain::begin() {
  randomSeed(micros());
  unsigned long now = millis();

  _x = (WALK_MIN_X + WALK_MAX_X) / 2;
  _walkTargetX = _x;
  _activity = ZeitoActivity::WANDER_IDLE;

  _nextWanderDecisionAt = now + randomInterval(WANDER_PAUSE_MIN_MS, WANDER_PAUSE_MAX_MS);
  _nextBlinkAt = now + randomInterval(BLINK_INTERVAL_MIN_MS, BLINK_INTERVAL_MAX_MS);
  // Let Zeito visit the plant/window reasonably soon after boot rather
  // than waiting a full cooldown period the very first time.
  _nextPlantEligibleAt = now + randomInterval(PLANT_VISIT_COOLDOWN_MIN_MS / 3, PLANT_VISIT_COOLDOWN_MAX_MS / 3);
  _nextWindowLookEligibleAt = now + randomInterval(WINDOW_LOOK_COOLDOWN_MIN_MS / 3, WINDOW_LOOK_COOLDOWN_MAX_MS / 3);
  _nextBedtimeAt = now + randomInterval(ACTIVE_PERIOD_MIN_MS, ACTIVE_PERIOD_MAX_MS);
  _lastInteractionAt = now;

  _view = ZeitoViewState();
}

// ===========================================================
// update() - called every loop(). No delay() anywhere below.
// ===========================================================
void ZeitoBrain::update() {
  unsigned long now = millis();

  updateBlink(now);
  checkBedtime(now);
  updateActivity(now);
  composeViewState(now);
}

SoundEvent ZeitoBrain::consumeSoundEvent() {
  SoundEvent e = _pendingSound;
  _pendingSound = SoundEvent::NONE;
  return e;
}

// ===========================================================
// Step: dispatch to the current activity's per-tick function
// ===========================================================
void ZeitoBrain::updateActivity(unsigned long now) {
  switch (_activity) {
    case ZeitoActivity::WANDER_IDLE: tickWanderIdle(now); break;

    case ZeitoActivity::WANDER_WALK:
    case ZeitoActivity::GO_TO_PLANT:
    case ZeitoActivity::GO_TO_BED:
    case ZeitoActivity::GO_TO_WINDOW:
    case ZeitoActivity::GO_TO_WINDOW_AUTO:
      tickWalking(now);
      break;

    case ZeitoActivity::WATERING: tickWatering(now); break;
    case ZeitoActivity::LOOKING_OUT_WINDOW: tickLookingOutWindow(now); break;
    case ZeitoActivity::SLEEPING: tickSleeping(now); break;
    case ZeitoActivity::WAKING_UP: tickWakingUp(now); break;
    case ZeitoActivity::GREETING: tickGreeting(now); break;

    case ZeitoActivity::REACT_TALK:
    case ZeitoActivity::REACT_PLAY:
    case ZeitoActivity::REACT_FEED:
      tickReaction(now);
      break;
  }
}

// ===========================================================
// Step: independent blink timer (paused while asleep - the sleeping
// pose already draws closed eyes)
// ===========================================================
void ZeitoBrain::updateBlink(unsigned long now) {
  if (_activity == ZeitoActivity::SLEEPING) {
    _blinking = false;
    return;
  }
  if (_blinking) {
    if (now - _blinkStartAt >= BLINK_DURATION_MS) {
      _blinking = false;
      _nextBlinkAt = now + randomInterval(BLINK_INTERVAL_MIN_MS, BLINK_INTERVAL_MAX_MS);
    }
  } else if (now >= _nextBlinkAt) {
    _blinking = true;
    _blinkStartAt = now;
  }
}

// ===========================================================
// Step: head to bed after a random stretch of active time (see
// HomeConfig::ACTIVE_PERIOD_MIN/MAX_MS) - no notion of "interaction"
// involved, this fires purely on its own. Only while idly wandering -
// a task in progress (watering, looking out the window...) is left to
// finish first.
// ===========================================================
void ZeitoBrain::checkBedtime(unsigned long now) {
  bool idling = (_activity == ZeitoActivity::WANDER_IDLE || _activity == ZeitoActivity::WANDER_WALK);
  if (idling && now >= _nextBedtimeAt) {
    startWalkTo(STAND_X_BED, ZeitoActivity::GO_TO_BED);
  }
}

// ===========================================================
// Final assembly for ZeitoView
// ===========================================================
void ZeitoBrain::composeViewState(unsigned long now) {
  _view.x = _x;
  _view.facingRight = _facingRight;
  _view.stepParity = _stepParity;

  bool isWalkingActivity = (_activity == ZeitoActivity::WANDER_WALK ||
                             _activity == ZeitoActivity::GO_TO_PLANT ||
                             _activity == ZeitoActivity::GO_TO_BED ||
                             _activity == ZeitoActivity::GO_TO_WINDOW ||
                             _activity == ZeitoActivity::GO_TO_WINDOW_AUTO);
  _view.isWalking = isWalkingActivity && (_x != _walkTargetX);

  _view.lying = (_activity == ZeitoActivity::SLEEPING);
  _view.eyesClosed = _blinking || _view.lying;
  _view.smiling = (_activity == ZeitoActivity::GREETING ||
                    _activity == ZeitoActivity::REACT_PLAY ||
                    _activity == ZeitoActivity::REACT_FEED ||
                    _activity == ZeitoActivity::WAKING_UP);
  _view.showWateringCan = (_activity == ZeitoActivity::WATERING);

  if (_view.lying) {
    // Z, then ZZ, then ZZZ - one more Z every ZZZ_STAGE_INTERVAL_MS
    // after falling asleep, capped at 3.
    unsigned long asleepFor = now - _stateEnteredAt;
    unsigned long stage = (asleepFor / ZZZ_STAGE_INTERVAL_MS) + 1;
    _view.zzzCount = (uint8_t)((stage > 3) ? 3 : stage);
  } else {
    _view.zzzCount = 0;
  }

  if (_view.lying && _dreamActive) {
    _view.showBubble = true;
    _view.bubbleIcon = _currentDreamIcon;
  } else if (_activity == ZeitoActivity::REACT_TALK) {
    _view.showBubble = true;
    _view.bubbleIcon = BubbleIcon::NOTE;
  } else {
    _view.showBubble = false;
    _view.bubbleIcon = BubbleIcon::NONE;
  }
}

// ===========================================================
// One function per activity's per-tick behavior
// ===========================================================
void ZeitoBrain::tickWanderIdle(unsigned long now) {
  if (now < _nextWanderDecisionAt) return;

  // One roll, split into ranges, so "water the plant" and "look out the
  // window" can never both win the same decision. Each option is only
  // in play once its own cooldown has passed - otherwise that slice of
  // the roll simply falls through to a plain wander step instead.
  bool canWater = (now >= _nextPlantEligibleAt);
  bool canLookOutWindow = (now >= _nextWindowLookEligibleAt);
  uint8_t roll = (uint8_t)random(100);

  if (canWater && roll < PLANT_VISIT_CHANCE_PERCENT) {
    startWalkTo(STAND_X_PLANT, ZeitoActivity::GO_TO_PLANT);
  } else if (canLookOutWindow && roll < PLANT_VISIT_CHANCE_PERCENT + WINDOW_LOOK_CHANCE_PERCENT) {
    startWalkTo(STAND_X_WINDOW, ZeitoActivity::GO_TO_WINDOW_AUTO);
  } else {
    int16_t target = (int16_t)random(WALK_MIN_X, WALK_MAX_X + 1);
    startWalkTo(target, ZeitoActivity::WANDER_WALK);
  }
}

void ZeitoBrain::tickWalking(unsigned long now) {
  if (!advanceWalk(now)) return; // still on the way

  switch (_activity) {
    case ZeitoActivity::WANDER_WALK:
      enterWanderIdle(now);
      break;

    case ZeitoActivity::GO_TO_PLANT:
      _activity = ZeitoActivity::WATERING;
      _stateEnteredAt = now;
      _stateDurationMs = randomInterval(WATERING_DURATION_MIN_MS, WATERING_DURATION_MAX_MS);
      _facingRight = false; // the pot is against the left wall
      raiseSound(SoundEvent::WATER);
      break;

    case ZeitoActivity::GO_TO_BED:
      _activity = ZeitoActivity::SLEEPING;
      _stateEnteredAt = now;
      _dreamActive = false;
      _nextDreamAt = now + randomInterval(DREAM_DELAY_MIN_MS, DREAM_DELAY_MAX_MS);
      _autoWakeAt = now + randomInterval(SLEEP_NAP_DURATION_MIN_MS, SLEEP_NAP_DURATION_MAX_MS);
      break;

    case ZeitoActivity::GO_TO_WINDOW:
      _activity = ZeitoActivity::GREETING;
      _stateEnteredAt = now;
      _facingRight = false; // the window is against the left wall
      raiseSound(SoundEvent::WELCOME);
      break;

    case ZeitoActivity::GO_TO_WINDOW_AUTO:
      _activity = ZeitoActivity::LOOKING_OUT_WINDOW;
      _stateEnteredAt = now;
      _stateDurationMs = randomInterval(WINDOW_LOOK_DURATION_MIN_MS, WINDOW_LOOK_DURATION_MAX_MS);
      _facingRight = false; // the window is against the left wall
      break;

    default:
      break;
  }
}

void ZeitoBrain::tickWatering(unsigned long now) {
  if (now - _stateEnteredAt >= _stateDurationMs) {
    _nextPlantEligibleAt = now + randomInterval(PLANT_VISIT_COOLDOWN_MIN_MS, PLANT_VISIT_COOLDOWN_MAX_MS);
    enterWanderIdle(now);
  }
}

void ZeitoBrain::tickLookingOutWindow(unsigned long now) {
  if (now - _stateEnteredAt >= _stateDurationMs) {
    _nextWindowLookEligibleAt = now + randomInterval(WINDOW_LOOK_COOLDOWN_MIN_MS, WINDOW_LOOK_COOLDOWN_MAX_MS);
    enterWanderIdle(now);
  }
}

void ZeitoBrain::tickSleeping(unsigned long now) {
  // The nap always ends on its own after _autoWakeAt, even with no
  // outside interaction at all (hand detection / a menu command can
  // still wake Zeito up earlier - see setHandPresence()/requestXxx()).
  if (now >= _autoWakeAt) {
    _activity = ZeitoActivity::WAKING_UP;
    _stateEnteredAt = now;
    _stateDurationMs = randomInterval(WAKING_UP_DURATION_MIN_MS, WAKING_UP_DURATION_MAX_MS);
    // A fresh bedtime target now, so Zeito gets a brand new random
    // ACTIVE_PERIOD_MIN/MAX_MS stretch before it heads back to bed -
    // without this it would immediately qualify for bedtime again on
    // its very next wander tick and barely get to do anything.
    _nextBedtimeAt = now + randomInterval(ACTIVE_PERIOD_MIN_MS, ACTIVE_PERIOD_MAX_MS);
    return;
  }

  if (!_dreamActive && now >= _nextDreamAt) {
    _dreamActive = true;
    _dreamStartAt = now;
    _currentDreamIcon = randomDreamIcon();
  } else if (_dreamActive && (now - _dreamStartAt >= DREAM_BUBBLE_DURATION_MS)) {
    _dreamActive = false;
    _nextDreamAt = now + randomInterval(DREAM_CYCLE_MIN_MS, DREAM_CYCLE_MAX_MS);
  }
}

void ZeitoBrain::tickWakingUp(unsigned long now) {
  if (now - _stateEnteredAt >= _stateDurationMs) {
    enterWanderIdle(now);
  }
}

void ZeitoBrain::tickGreeting(unsigned long now) {
  // Smile for at least GREETING_DURATION_MS (so even a brief wave gets
  // a proper greeting), then keep waiting as long as the hand is still
  // there, and only head back to the normal routine once it's gone.
  // GREETING_MAX_DURATION_MS is a hard cap so a hand that never leaves
  // (or a sensor stuck reading "near") can't hold this state forever.
  bool minDurationElapsed = (now - _stateEnteredAt >= GREETING_DURATION_MS);
  bool maxDurationElapsed = (now - _stateEnteredAt >= GREETING_MAX_DURATION_MS);
  if (maxDurationElapsed || (minDurationElapsed && !_handNear)) {
    enterWanderIdle(now);
  }
}

void ZeitoBrain::tickReaction(unsigned long now) {
  if (now - _stateEnteredAt >= REACTION_DURATION_MS) {
    enterWanderIdle(now);
  }
}

// ===========================================================
// Public hooks: inputs from the outside world
// ===========================================================
void ZeitoBrain::setHandPresence(bool near) {
  bool risingEdge = near && !_handNear;
  _handNear = near;
  if (!risingEdge) return; // tickGreeting() watches _handNear directly for the departure side

  unsigned long now = millis();
  if (now < _handCooldownUntil) return; // sensor jitter right at the threshold
  // Already on its way to (or standing at) the window - don't restart.
  if (_activity == ZeitoActivity::GO_TO_WINDOW || _activity == ZeitoActivity::GREETING) return;

  _handCooldownUntil = now + HAND_DETECT_COOLDOWN_MS;
  _dreamActive = false;
  startWalkTo(STAND_X_WINDOW, ZeitoActivity::GO_TO_WINDOW);
  _lastInteractionAt = now;
}

void ZeitoBrain::notifyInteraction() {
  _lastInteractionAt = millis();
}

void ZeitoBrain::requestTalk() {
  unsigned long now = millis();
  _activity = ZeitoActivity::REACT_TALK;
  _stateEnteredAt = now;
  raiseSound(SoundEvent::TALK);
  _lastInteractionAt = now;
}

void ZeitoBrain::requestSleep() {
  unsigned long now = millis();
  if (_activity != ZeitoActivity::SLEEPING && _activity != ZeitoActivity::GO_TO_BED) {
    startWalkTo(STAND_X_BED, ZeitoActivity::GO_TO_BED);
  }
  _lastInteractionAt = now;
}

void ZeitoBrain::requestPlay() {
  unsigned long now = millis();
  _activity = ZeitoActivity::REACT_PLAY;
  _stateEnteredAt = now;
  raiseSound(SoundEvent::PLAY);
  _lastInteractionAt = now;
}

void ZeitoBrain::requestFeed() {
  unsigned long now = millis();
  _activity = ZeitoActivity::REACT_FEED;
  _stateEnteredAt = now;
  raiseSound(SoundEvent::FEED);
  _lastInteractionAt = now;
}

// ===========================================================
// Small helpers
// ===========================================================
void ZeitoBrain::startWalkTo(int16_t targetX, ZeitoActivity activity) {
  _walkTargetX = targetX;
  _activity = activity;
  _lastStepAt = millis();
}

bool ZeitoBrain::advanceWalk(unsigned long now) {
  if (_x == _walkTargetX) return true;
  if (now - _lastStepAt >= WALK_STEP_INTERVAL_MS) {
    _lastStepAt = now;
    if (_walkTargetX > _x) {
      _x++;
      _facingRight = true;
    } else {
      _x--;
      _facingRight = false;
    }
    _stepParity = !_stepParity;
  }
  return _x == _walkTargetX;
}

void ZeitoBrain::enterWanderIdle(unsigned long now, unsigned long extraDelayMs) {
  _activity = ZeitoActivity::WANDER_IDLE;
  _nextWanderDecisionAt = now + randomInterval(WANDER_PAUSE_MIN_MS, WANDER_PAUSE_MAX_MS) + extraDelayMs;
}

void ZeitoBrain::raiseSound(SoundEvent event) {
  _pendingSound = event;
}

unsigned long ZeitoBrain::randomInterval(unsigned long minMs, unsigned long maxMs) {
  return (unsigned long)random((long)minMs, (long)maxMs + 1);
}

BubbleIcon ZeitoBrain::randomDreamIcon() {
  // BubbleIcon::STAR .. BubbleIcon::PIZZA are the dream-worthy icons;
  // NONE/NOTE/COUNT are not dreams (NOTE is reserved for "Talk").
  uint8_t first = (uint8_t)BubbleIcon::STAR;
  uint8_t last = (uint8_t)BubbleIcon::PIZZA;
  return (BubbleIcon)random(first, last + 1);
}
