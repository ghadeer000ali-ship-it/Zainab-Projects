/*
 * ZeitoHome.h
 * -----------
 * Top-level facade: owns every subsystem (room, character, brain,
 * sensors, sound, menu) and is the one object the .ino actually talks
 * to. This is the intended integration point for future peripherals -
 * new hardware gets a poller here, not a rewrite of ZeitoBrain/ZeitoView.
 */
#ifndef ZEITO_HOME_H
#define ZEITO_HOME_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RoomView.h"
#include "ZeitoView.h"
#include "ZeitoBrain.h"
#include "UltrasonicSensor.h"
#include "Joystick.h"
#include "SoundFX.h"
#include "MenuView.h"

class ZeitoHome {
public:
  explicit ZeitoHome(Adafruit_SSD1306 &display);

  // Call once from setup(), after display.begin() has succeeded.
  void begin();

  // Call every loop() iteration.
  void update();

private:
  Adafruit_SSD1306 *_display;

  RoomView _room;
  ZeitoView _zeitoView;
  ZeitoBrain _brain;
  SoundFX _sound;

  // Only touched while HomeConfig::ENABLE_HC_SR04 is true (see
  // handleUltrasonic()/begin()/update()) - kept as a member, untouched
  // otherwise, so flipping that flag back on needs no other changes.
  UltrasonicSensor _sonar;

  // Only touched while HomeConfig::ENABLE_JOYSTICK_MENU is true (see
  // handleJoystick()/begin()/render()) - kept as members, untouched
  // otherwise, so flipping that flag back on needs no other changes.
  Joystick _joystick;
  MenuView _menu;

  unsigned long _lastRenderMs = 0;

  void handleUltrasonic(unsigned long now);
  void handleJoystick(unsigned long now);
  void handleBrainSound(unsigned long now);
  void render(unsigned long now);
};

#endif // ZEITO_HOME_H
