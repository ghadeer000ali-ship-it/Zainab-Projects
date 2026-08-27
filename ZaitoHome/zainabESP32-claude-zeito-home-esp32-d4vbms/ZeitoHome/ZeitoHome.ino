/*
 * ZeitoHome.ino
 * -------------
 * "Zeito Home" - a small pixel-art character named زيتو living in a
 * one-room house drawn on a 128x64 SSD1306 I2C OLED, powered by an
 * ESP32. Zeito wanders the room, waters a plant, naps on a bed and
 * dreams, and greets you at the window when your hand gets close to
 * the HC-SR04. A joystick opens a small Talk/Sleep/Play/Feed menu.
 *
 * The whole project runs on millis()-based state machines - there is
 * no delay() anywhere in the sketch or its supporting files, so the
 * display, sensor polling and buzzer all stay responsive every loop().
 *
 * This file only does three things: bring up the display and sensors,
 * construct a ZeitoHome, and pump ZeitoHome::update() from loop(). All
 * of the actual behavior (ZeitoBrain), drawing (RoomView/ZeitoView) and
 * hardware drivers (UltrasonicSensor/Joystick/SoundFX/MenuView) live in
 * the other files in this sketch folder - see README.md for the full
 * project map.
 *
 * WIRING
 * -------
 *   OLED SSD1306 (I2C)   SDA  -> GPIO 21
 *                         SCL  -> GPIO 22
 *                         VCC  -> 3V3
 *                         GND  -> GND
 *
 *   HC-SR04               TRIG -> GPIO 18
 *                         ECHO -> GPIO 19 (5V modules: use a divider to 3.3V)
 *                         VCC  -> 5V
 *                         GND  -> GND
 *
 *   Joystick               VRX -> GPIO 34
 *                         VRY -> GPIO 35
 *                         SW  -> GPIO 27
 *                         VCC -> 3V3
 *                         GND -> GND
 *
 *   Buzzer                 +   -> GPIO 26
 *                         -   -> GND
 *
 * Required libraries (Library Manager):
 *   - Adafruit GFX Library
 *   - Adafruit SSD1306
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HomeConfig.h"
#include "ZeitoHome.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ZeitoHome zeitoHome(display);

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) {
      // Halt here rather than looping into an undefined display state.
      // (No delay() needed - we're not doing anything else anyway.)
    }
  }

  display.clearDisplay();
  display.display();

  zeitoHome.begin();
}

void loop() {
  zeitoHome.update();

  // ---------------------------------------------------------------
  // FUTURE PERIPHERALS go here, as non-blocking pollers, following the
  // same pattern as the sensors already wired above (see README.md ->
  // "How to add a new feature"): read hardware, then call one of
  // ZeitoHome's existing public methods, or add a new one that forwards
  // to ZeitoBrain - never touch ZeitoBrain.cpp/ZeitoView.cpp directly.
  // ---------------------------------------------------------------
}
