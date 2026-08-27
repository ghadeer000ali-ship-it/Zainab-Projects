/*
 * ArabicOLED_Demo.ino
 * --------------------
 * Demonstrates the ArabicOLED library: prints three Arabic strings,
 * right-to-left with automatic letter joining, to a 128x64 SSD1306
 * I2C OLED on an ESP32.
 *
 *   مرحبا       (Marhaba   - "Hello")
 *   زيتونه      (Zaytouna  - "Olive")
 *   كيف حالك؟   (Kayfa haluk? - "How are you?")
 *
 * WIRING (ESP32 default I2C pins)
 * ---------------------------------
 *   OLED SDA -> GPIO 21
 *   OLED SCL -> GPIO 22
 *   OLED VCC -> 3V3
 *   OLED GND -> GND
 *
 * This file is saved as UTF-8 (the standard encoding for .ino files),
 * so the Arabic string literals below are plain UTF-8 bytes - no
 * special escaping needed.
 *
 * Required libraries (install via Library Manager):
 *   - Adafruit GFX Library
 *   - Adafruit SSD1306
 *   - ArabicOLED (this library)
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArabicOLED.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1     // No dedicated reset pin on most SSD1306 modules
#define OLED_I2C_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ArabicOLED arabic(display);

// The three words/phrase to cycle through.
const char *const kWords[] = {
  "مرحبا",
  "زيتونه",
  "كيف حالك؟",
};
const uint8_t kWordCount = sizeof(kWords) / sizeof(kWords[0]);

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.display();
  delay(500);
}

void loop() {
  for (uint8_t i = 0; i < kWordCount; i++) {
    display.clearDisplay();

    // Big, single word/phrase centered on screen.
    arabic.printCentered(kWords[i], 20);

    display.display();
    delay(2000);
  }

  // Also show all three at once, right-aligned and stacked, to
  // demonstrate multi-line layout with drawText(). Each glyph is
  // ArabicOLED::glyphHeight() (24px) tall; three lines don't fit on a
  // 64px display without slight overlap, so we pack them at a tighter
  // pitch than the full glyph height - the letters themselves have
  // enough blank margin that this stays readable.
  display.clearDisplay();
  int16_t xRight = display.width() - 1;
  const int16_t pitch = display.height() / kWordCount; // 64 / 3 = 21px
  for (uint8_t i = 0; i < kWordCount; i++) {
    arabic.drawText(kWords[i], xRight, i * pitch);
  }
  display.display();
  delay(3000);
}
