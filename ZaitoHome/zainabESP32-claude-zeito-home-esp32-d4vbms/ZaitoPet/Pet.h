/*
 * Pet.h
 * -----
 * Top-level facade: owns a PetBrain (behavior) and a PetFace (drawing),
 * and is the one object the main sketch actually talks to. This is
 * also the intended integration point for future peripherals - see the
 * project README's "adding a new feature" walkthrough.
 */
#ifndef PET_H
#define PET_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PetBrain.h"
#include "PetFace.h"

class Pet {
public:
  explicit Pet(Adafruit_SSD1306 &display);

  // Call once from setup(), after display.begin() has succeeded.
  void begin();

  // Call every loop() iteration. Internally throttles actual redraws to
  // PetConfig::FRAME_INTERVAL - safe, and intended, to call this as
  // often as loop() runs.
  void update();

  // ---- Forwarded hooks for future peripherals ----
  // A button/touch ISR or poller in the .ino can call this directly;
  // Pet doesn't need to know what triggered it.
  void notifyInteraction() { _brain.notifyInteraction(); }

  // Temporarily override Zaito's mood - e.g. a future battery monitor
  // calling pet.forceExpression(Expression::SAD, 3000) when the battery
  // gets low.
  void forceExpression(Expression e, unsigned long durationMs) {
    _brain.forceExpression(e, durationMs);
  }

private:
  Adafruit_SSD1306 *_display;
  PetBrain _brain;
  PetFace _face;
  unsigned long _lastRenderMs = 0;
};

#endif // PET_H
