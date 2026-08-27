/*
 * Joystick.h
 * ----------
 * Non-blocking, debounced joystick reader. The joystick never moves
 * Zeito directly - it only ever produces UI events (up/down/left/right/
 * pressed) that MenuView and the top-level facade react to.
 *
 * Each axis fires one edge event the moment it crosses the deadzone
 * threshold, then repeats every HomeConfig::JOYSTICK_REPEAT_MS while
 * held past the threshold (handy for scrolling a menu), and goes quiet
 * again once the stick returns to center.
 */
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

struct JoystickEvent {
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool pressed = false; // button edge (just went down), already debounced

  bool any() const { return up || down || left || right || pressed; }
};

class Joystick {
public:
  void begin();

  // Call every loop() iteration. Returns the events that fired this call.
  JoystickEvent update(unsigned long now);

private:
  // Per-axis edge/repeat state, shared logic for VRX and VRY.
  struct AxisState {
    bool neutral = true;
    unsigned long lastFireAt = 0;
  };
  AxisState _xAxis, _yAxis;

  bool _buttonWasDown = false;
  unsigned long _lastButtonChangeAt = 0;

  // Polls one axis; sets negOut/posOut if a negative/positive-direction
  // event fires this call (edge, or repeat while held).
  void pollAxis(int16_t value, AxisState &state, unsigned long now, bool &negOut, bool &posOut);
};

#endif // JOYSTICK_H
