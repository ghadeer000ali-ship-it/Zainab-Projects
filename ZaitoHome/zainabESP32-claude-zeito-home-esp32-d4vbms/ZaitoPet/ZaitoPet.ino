/*
 * ZaitoPet.ino
 * ------------
 * "زيتو" (Zaito) - a virtual pet character living on a 128x64 SSD1306
 * I2C OLED, powered by an ESP32.
 *
 * This file only does three things: bring up the display, construct a
 * Pet, and pump Pet::update() from loop(). All of the actual behavior
 * (PetBrain) and drawing (PetFace) code lives in the other files in
 * this sketch folder - see README.md for the full project map.
 *
 * WIRING (ESP32 default I2C pins)
 * ---------------------------------
 *   OLED SDA -> GPIO 21
 *   OLED SCL -> GPIO 22
 *   OLED VCC -> 3V3
 *   OLED GND -> GND
 *
 * Required libraries (Library Manager):
 *   - Adafruit GFX Library
 *   - Adafruit SSD1306
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PetConfig.h"
#include "Pet.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Pet zaito(display);

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) {
      // Halt here rather than looping into undefined display state.
      // (No delay() needed - we're not doing anything else anyway.)
    }
  }

  display.clearDisplay();
  display.display();

  zaito.begin();
}

void loop() {
  zaito.update();

  // ---------------------------------------------------------------
  // FUTURE PERIPHERALS go here, as non-blocking pollers, e.g.:
  //
  //   if (digitalRead(PetConfig::BUTTON_PIN) == LOW) zaito.notifyInteraction();
  //   if (touchRead(PetConfig::TOUCH_PIN) < TOUCH_THRESHOLD) zaito.notifyInteraction();
  //
  // See README.md -> "Adding a new feature" for the full walkthrough
  // (buttons, touch, Wi-Fi, clock, weather, battery, games, menus,
  // sound effects...) - none of it requires touching PetBrain.cpp or
  // PetFace.cpp.
  // ---------------------------------------------------------------
}
