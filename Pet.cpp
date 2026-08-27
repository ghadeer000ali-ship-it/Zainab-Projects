/*
 * Pet.cpp
 * -------
 * Wires PetBrain's output into PetFace's input, on a throttled render
 * clock. This is the only place that calls display.clearDisplay() /
 * display.display() - PetFace only ever draws into the existing buffer.
 */

#include "Pet.h"
#include "PetConfig.h"

Pet::Pet(Adafruit_SSD1306 &display) : _display(&display), _face(display) {}

void Pet::begin() {
  _brain.begin();
  _lastRenderMs = millis();
}

void Pet::update() {
  // The behavior model updates every call - it's cheap (a handful of
  // millis() comparisons) and needs fine-grained timing for smooth
  // animation, independent of how often we actually redraw the OLED.
  _brain.update();

  unsigned long now = millis();
  if (now - _lastRenderMs < PetConfig::FRAME_INTERVAL) return;
  _lastRenderMs = now;

  _display->clearDisplay();
  _face.render(_brain.getFaceState());
  _display->display();
}
