/*
 * ZeitoHome.cpp
 * -------------
 * Wires every subsystem's input/output into the others. This is the
 * only place that calls display.clearDisplay()/display.display(), and
 * the only place that turns a ZeitoBrain::SoundEvent or a joystick
 * event into an actual buzzer call or a ZeitoBrain::requestXxx() call.
 */

#include "ZeitoHome.h"
#include "HomeConfig.h"

using namespace HomeConfig;

ZeitoHome::ZeitoHome(Adafruit_SSD1306 &display)
    : _display(&display), _room(display), _zeitoView(display), _menu(display) {}

void ZeitoHome::begin() {
  _brain.begin();
  _sound.begin();
  if (ENABLE_HC_SR04) _sonar.begin();
  if (ENABLE_JOYSTICK_MENU) _joystick.begin();
  _lastRenderMs = millis();
}

void ZeitoHome::update() {
  unsigned long now = millis();

  if (ENABLE_HC_SR04) handleUltrasonic(now);
  if (ENABLE_JOYSTICK_MENU) handleJoystick(now);

  _brain.update();
  handleBrainSound(now);

  _sound.update(now);
  if (ENABLE_JOYSTICK_MENU) _menu.update(now);

  render(now);
}

// ===========================================================
// HC-SR04. Disabled while HomeConfig::ENABLE_HC_SR04 is false (the
// default in this version - see the guards in begin()/update()) - this
// function is simply never called then, so _sonar is never pinged and
// ZeitoBrain::setHandPresence() is never invoked: nothing waits on or
// branches on hand proximity, and Zeito's routine runs entirely off
// its own timers. Left intact so re-enabling the flag brings hand
// detection back with no other changes.
//
// When enabled, ZeitoBrain itself edge-detects "something just
// arrived" (to wake up / walk to the window) and level-detects "still
// there" (to decide when to head back to its normal routine) - see
// ZeitoBrain::setHandPresence().
// ===========================================================
void ZeitoHome::handleUltrasonic(unsigned long now) {
  _sonar.update(now);
  _brain.setHandPresence(_sonar.isNear(HAND_DETECT_DISTANCE_CM));
}

// ===========================================================
// Joystick: never drives Zeito directly. First deflection opens the
// popup menu; while it's open, up/down browse it, the button confirms,
// and left cancels.
//
// Disabled while HomeConfig::ENABLE_JOYSTICK_MENU is false (see the
// guards in begin()/update()/render()) - this function is simply never
// called then, left intact so re-enabling the flag brings the whole
// menu back with no other changes.
// ===========================================================
void ZeitoHome::handleJoystick(unsigned long now) {
  JoystickEvent ev = _joystick.update(now);
  if (!ev.any()) return;

  if (!_menu.isOpen()) {
    _menu.open(now);
    _brain.notifyInteraction();
    return;
  }

  if (ev.up) {
    _menu.navigateUp(now);
    _sound.playClick();
  }
  if (ev.down) {
    _menu.navigateDown(now);
    _sound.playClick();
  }
  if (ev.left) {
    _menu.close();
  }
  if (ev.pressed) {
    MenuAction action = _menu.confirmSelection(now);
    _menu.close();
    _sound.playSelect();

    switch (action) {
      case MenuAction::TALK:  _brain.requestTalk();  break;
      case MenuAction::SLEEP: _brain.requestSleep(); break;
      case MenuAction::PLAY:  _brain.requestPlay();  break;
      case MenuAction::FEED:  _brain.requestFeed();  break;
      default: break;
    }
  }
}

// ===========================================================
// ZeitoBrain never touches the buzzer - it only raises a SoundEvent.
// This is the one place that turns that into an actual sound.
// ===========================================================
void ZeitoHome::handleBrainSound(unsigned long now) {
  (void)now;
  switch (_brain.consumeSoundEvent()) {
    case SoundEvent::WATER:   _sound.playWater();   break;
    case SoundEvent::WELCOME: _sound.playWelcome(); break;
    case SoundEvent::TALK:    _sound.playTalk();    break;
    case SoundEvent::PLAY:    _sound.playPlay();    break;
    case SoundEvent::FEED:    _sound.playFeed();    break;
    default: break;
  }
}

// ===========================================================
// Render, throttled to HomeConfig::FRAME_INTERVAL_MS.
// ===========================================================
void ZeitoHome::render(unsigned long now) {
  if (now - _lastRenderMs < FRAME_INTERVAL_MS) return;
  _lastRenderMs = now;

  _display->clearDisplay();
  _room.draw();
  _zeitoView.draw(_brain.getViewState());
  if (ENABLE_JOYSTICK_MENU) _menu.draw();
  _display->display();
}
