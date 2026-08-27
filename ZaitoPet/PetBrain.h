/*
 * PetBrain.h
 * ----------
 * The BEHAVIOR layer. PetBrain decides what Zaito is doing right now -
 * blinking, looking around, changing mood, yawning, falling asleep -
 * and writes the result into a FaceState every update(). It never
 * touches the display directly; PetFace is the only thing that reads
 * its output.
 *
 * Every timer here is millis()-based and non-blocking: update() must be
 * called every loop() iteration, and it always returns immediately.
 * There is no delay() anywhere in this class.
 */
#ifndef PET_BRAIN_H
#define PET_BRAIN_H

#include <Arduino.h>
#include "PetTypes.h"

class PetBrain {
public:
  PetBrain();

  // Call once from setup() (after Serial/randomSeed are ready).
  void begin();

  // Call every loop() iteration. Cheap - just a handful of millis()
  // comparisons - safe to call as often as you like.
  void update();

  // What PetFace should draw right now.
  const FaceState &getFaceState() const { return _face; }

  // ---- Hooks reserved for future input sources -------------------
  // These exist today so that wiring up a button/touch sensor/BLE
  // command later means calling one of these from the .ino, not
  // reworking the state machine. See the project README.
  void notifyInteraction();                 // e.g. a button/touch tap
  void forceExpression(Expression e, unsigned long durationMs);

private:
  FaceState _face;
  PetState _petState = PetState::AWAKE;

  Expression _currentExpression = Expression::NORMAL;
  Expression _previousExpression = Expression::NORMAL; // avoids picking the same mood twice in a row

  // A forced expression (from forceExpression()) temporarily overrides
  // the random mood scheduler.
  bool _expressionForced = false;
  unsigned long _forcedUntilMs = 0;

  // ---- Discrete transient animations (mutually exclusive: only one
  // of blink/wink/look-around/yawn plays at a time, so they never fight
  // over the same eyes). ----
  enum class TransientAnim : uint8_t { NONE, BLINK, WINK, LOOK_AROUND, YAWN };
  TransientAnim _activeAnim = TransientAnim::NONE;
  unsigned long _animStartMs = 0;
  unsigned long _animDurationMs = 0;
  int8_t _winkEye = -1;      // -1 = left, +1 = right (which eye winks)
  int8_t _lookTargetX = 0;   // look-around destination, pixels
  int8_t _lookTargetY = 0;

  // ---- One-shot reaction bursts, triggered on entering a mood ----
  bool _jumpActive = false;
  unsigned long _jumpStartMs = 0;
  bool _shakeActive = false;
  unsigned long _shakeStartMs = 0;

  // ---- Overlay icon burst ----
  OverlayType _overlay = OverlayType::NONE;
  unsigned long _overlayStartMs = 0;
  unsigned long _overlayDurationMs = 0;

  // ---- Sleep transition (eases eyelids shut/open instead of snapping) ----
  bool _sleepTransitionActive = false;
  unsigned long _sleepTransitionStartMs = 0;
  float _sleepTransitionFrom = 1.0f;
  float _sleepTransitionTo = 1.0f;

  // ---- Independent random schedule timers (each stores its NEXT
  // fire time; on firing, the matching trigger_xxx() runs and a new
  // random interval is rolled). ----
  unsigned long _nextBlinkAt = 0;
  unsigned long _nextLookAt = 0;
  unsigned long _nextYawnAt = 0;
  unsigned long _nextExpressionChangeAt = 0;
  unsigned long _nextSleepAt = 0;
  unsigned long _nextWakeAt = 0;
  unsigned long _nextSweatAt = 0;

  // ---- Per-tick update steps (called from update() every loop) ----
  void updateScheduledTimers(unsigned long now);
  void updateActiveAnimation(unsigned long now);
  void updateReactionBursts(unsigned long now);
  void updateOverlay(unsigned long now);
  void updateBreathing(unsigned long now);
  void updateSleepTransition(unsigned long now);
  void composeFaceState();

  // ---- One independent function per animation/trigger ----
  void triggerBlink(unsigned long now);
  void triggerWink(unsigned long now);
  void triggerLookAround(unsigned long now);
  void triggerYawn(unsigned long now);
  void triggerJump(unsigned long now);
  void triggerShake(unsigned long now);
  void triggerSweatFlash(unsigned long now);
  void triggerOverlay(OverlayType type, unsigned long now, unsigned long durationMs);
  void enterSleep(unsigned long now);
  void wakeUp(unsigned long now);
  void changeExpression(Expression next, unsigned long now);

  // ---- Small helpers ----
  Expression pickRandomExpression();
  static unsigned long randomInterval(unsigned long minMs, unsigned long maxMs);
  static float easeEnvelope(float t); // 0 -> 1 -> 0 triangular-ish ease, t in [0,1]
};

#endif // PET_BRAIN_H
