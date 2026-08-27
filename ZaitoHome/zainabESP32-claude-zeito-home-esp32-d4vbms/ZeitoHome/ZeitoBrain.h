/*
 * ZeitoBrain.h
 * ------------
 * The AI / BEHAVIOR layer. Decides what Zeito is doing right now -
 * wandering, watering the plant, looking out the window, sleeping and
 * dreaming (then waking up smiling, all on its own timers), greeting a
 * hand at the window, or reacting to a menu command - and writes the
 * result into a ZeitoViewState every update(). No activity lasts
 * forever: every one of them has a timer, an arrival, or an external
 * event that moves it on to the next. In this version
 * (HomeConfig::ENABLE_HC_SR04 / ENABLE_JOYSTICK_MENU both false),
 * Zeito needs no outside input at all - the whole wander/plant/window/
 * bed cycle runs purely off its own timers. It never touches the
 * display or the buzzer directly:
 *   - ZeitoView reads getViewState() to draw.
 *   - The sketch/facade reads consumeSoundEvent() to play a sound.
 *
 * Every timer is millis()-based and non-blocking: update() must be
 * called every loop() iteration and always returns immediately. There
 * is no delay() anywhere in this class.
 */
#ifndef ZEITO_BRAIN_H
#define ZEITO_BRAIN_H

#include <Arduino.h>
#include "HomeTypes.h"

class ZeitoBrain {
public:
  ZeitoBrain();

  // Call once from setup().
  void begin();

  // Call every loop() iteration.
  void update();

  // What ZeitoView should draw right now.
  const ZeitoViewState &getViewState() const { return _view; }

  // Returns the sound Zeito wants played since the last call, then
  // clears it back to SoundEvent::NONE. Call this once per loop().
  SoundEvent consumeSoundEvent();

  // ---- Inputs from the outside world -----------------------------
  // Continuous HC-SR04 reading: call every loop() with whether
  // something is currently closer than HomeConfig::HAND_DETECT_DISTANCE_CM.
  // A false->true edge wakes Zeito up if asleep and sends it to the
  // window to say hello; Zeito then keeps smiling at the window until
  // this goes back to false (the hand moves away), then returns to its
  // normal routine. Unused while HomeConfig::ENABLE_HC_SR04 is false (the
  // default in this version) - nothing calls this, so nothing ever
  // waits on or branches on hand proximity; kept intact so re-enabling
  // the sensor is just flipping that flag back on.
  void setHandPresence(bool near);

  // Any interaction that should merely reset the "how long since
  // anything happened" idle/sleep timer, without changing behavior
  // (e.g. just opening the menu). Unused while the menu is disabled,
  // kept so re-enabling it (see HomeConfig::ENABLE_JOYSTICK_MENU) needs
  // no changes here.
  void notifyInteraction();

  // ---- Menu commands (Talk / Sleep / Play / Feed) -----------------
  // Unused while HomeConfig::ENABLE_JOYSTICK_MENU is false - kept
  // intact so re-enabling the menu is just flipping that flag back on.
  void requestTalk();
  void requestSleep();
  void requestPlay();
  void requestFeed();

private:
  ZeitoActivity _activity = ZeitoActivity::WANDER_IDLE;
  ZeitoViewState _view;
  SoundEvent _pendingSound = SoundEvent::NONE;

  // ---- Position / walking ----
  int16_t _x;
  int16_t _walkTargetX;
  bool _facingRight = true;
  bool _stepParity = false;
  unsigned long _lastStepAt = 0;

  // ---- Generic "how long have we been in this activity" clock, and
  // the randomly-picked duration (when the current activity is a timed
  // "stand here for a while" one - watering, looking out the window,
  // waking up) ----
  unsigned long _stateEnteredAt = 0;
  unsigned long _stateDurationMs = 0;

  // ---- Blinking (independent of activity, except while asleep) ----
  bool _blinking = false;
  unsigned long _blinkStartAt = 0;
  unsigned long _nextBlinkAt = 0;

  // ---- Idle wandering ----
  unsigned long _nextWanderDecisionAt = 0;

  // ---- Watering the plant ----
  unsigned long _nextPlantEligibleAt = 0;

  // ---- Looking out the window on its own ----
  unsigned long _nextWindowLookEligibleAt = 0;

  // ---- Bedtime: a fresh random target is picked every time Zeito
  // wakes up, so it stays up for a random ACTIVE_PERIOD_MIN/MAX_MS
  // stretch each time, with no notion of "interaction" involved at all.
  unsigned long _nextBedtimeAt = 0;

  // ---- Sleep / dreaming ----
  unsigned long _lastInteractionAt = 0; // only used by the hand/menu hooks above - unused while both are disabled
  unsigned long _autoWakeAt = 0; // nap ends here even with no external wake-up
  bool _dreamActive = false;
  unsigned long _dreamStartAt = 0;
  unsigned long _nextDreamAt = 0;
  BubbleIcon _currentDreamIcon = BubbleIcon::STAR;

  // ---- Hand presence (HC-SR04) ----
  bool _handNear = false;
  unsigned long _handCooldownUntil = 0; // guards against sensor jitter right at the threshold

  // ---- Per-tick update steps ----
  void updateActivity(unsigned long now);
  void updateBlink(unsigned long now);
  void checkBedtime(unsigned long now);
  void composeViewState(unsigned long now);

  // ---- One function per activity's per-tick behavior ----
  void tickWanderIdle(unsigned long now);
  void tickWalking(unsigned long now); // shared by every "walking toward _walkTargetX" activity
  void tickWatering(unsigned long now);
  void tickLookingOutWindow(unsigned long now);
  void tickSleeping(unsigned long now);
  void tickWakingUp(unsigned long now);
  void tickGreeting(unsigned long now);
  void tickReaction(unsigned long now);

  // ---- Helpers ----
  void startWalkTo(int16_t targetX, ZeitoActivity activity);
  bool advanceWalk(unsigned long now); // moves _x one pixel closer if it's time; returns "arrived"
  void enterWanderIdle(unsigned long now, unsigned long extraDelayMs = 0);
  void raiseSound(SoundEvent event);
  static unsigned long randomInterval(unsigned long minMs, unsigned long maxMs);
  static BubbleIcon randomDreamIcon();
};

#endif // ZEITO_BRAIN_H
