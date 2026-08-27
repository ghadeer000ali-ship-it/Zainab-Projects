#include "Joystick.h"
#include "HomeConfig.h"

using namespace HomeConfig;

void Joystick::begin() {
  pinMode(PIN_JOYSTICK_SW, INPUT_PULLUP); // most joystick modules pull SW low when pressed
}

void Joystick::pollAxis(int16_t value, AxisState &state, unsigned long now, bool &negOut, bool &posOut) {
  int16_t offset = value - JOYSTICK_CENTER;

  if (abs(offset) < JOYSTICK_THRESHOLD) {
    state.neutral = true;
    return;
  }

  bool fire = false;
  if (state.neutral) {
    // Fresh deflection from center: fire immediately.
    fire = true;
    state.neutral = false;
  } else if (now - state.lastFireAt >= JOYSTICK_REPEAT_MS) {
    // Held past the threshold: auto-repeat.
    fire = true;
  }

  if (fire) {
    state.lastFireAt = now;
    if (offset < 0) negOut = true;
    else posOut = true;
  }
}

JoystickEvent Joystick::update(unsigned long now) {
  JoystickEvent ev;

  int16_t vrx = analogRead(PIN_JOYSTICK_VRX);
  int16_t vry = analogRead(PIN_JOYSTICK_VRY);

  pollAxis(vrx, _xAxis, now, ev.left, ev.right);
  pollAxis(vry, _yAxis, now, ev.up, ev.down);

  bool buttonDown = (digitalRead(PIN_JOYSTICK_SW) == LOW);
  if (buttonDown != _buttonWasDown && (now - _lastButtonChangeAt) >= BUTTON_DEBOUNCE_MS) {
    _lastButtonChangeAt = now;
    _buttonWasDown = buttonDown;
    if (buttonDown) ev.pressed = true; // report the press edge only
  }

  return ev;
}
