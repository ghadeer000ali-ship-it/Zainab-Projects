/*
 * ArabicOLED.h
 * ------------
 * Arabic-text renderer for SSD1306-based 128x64 I2C OLED displays on
 * ESP32, built on top of Adafruit_GFX + Adafruit_SSD1306.
 *
 * WHAT THIS LIBRARY DOES
 * -----------------------
 *  - Accepts ordinary UTF-8 Arabic text (the same bytes your source file
 *    already contains if you type Arabic between quotes, since Arduino
 *    IDE / PlatformIO save .ino/.cpp files as UTF-8).
 *  - Automatically works out which shape each letter needs - isolated,
 *    initial, medial or final - based on its neighbours, exactly like a
 *    word processor does. You never specify letter shapes yourself.
 *  - Draws the text right-to-left, as Arabic is written.
 *  - Falls back to Adafruit_GFX's built-in font for any non-Arabic
 *    characters (digits, Latin letters, etc.) so mixed text doesn't
 *    just disappear.
 *
 * WHAT THIS LIBRARY DOES NOT DO
 * -------------------------------
 *  - Full Unicode Bidirectional Algorithm (UAX #9). Arabic runs are laid
 *    out correctly; embedded Latin/number runs are drawn left-to-right
 *    internally, but if you mix multiple separate Latin words into an
 *    Arabic sentence they will still appear in right-to-left *order*
 *    relative to each other. For simple cases (a single number or word
 *    embedded in an Arabic sentence) this looks correct; for full mixed
 *    paragraphs it is only an approximation.
 *  - Arabic diacritics (harakat/tashkeel) - only base letters are
 *    supported.
 *
 * USAGE
 * -----
 *   #include <Wire.h>
 *   #include <Adafruit_GFX.h>
 *   #include <Adafruit_SSD1306.h>
 *   #include <ArabicOLED.h>
 *
 *   Adafruit_SSD1306 display(128, 64, &Wire, -1);
 *   ArabicOLED arabic(display);
 *
 *   void setup() {
 *     display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 *     display.clearDisplay();
 *     arabic.printLine("مرحبا", 0);
 *     display.display();
 *   }
 *
 * See examples/ArabicOLED_Demo for a complete sketch.
 */
#ifndef ARABIC_OLED_H
#define ARABIC_OLED_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class ArabicOLED {
public:
  // `display` must already have had display.begin(...) called on it
  // before you draw with this class (the library does not own the I2C
  // bus setup, since that varies by board/reset-pin/address).
  explicit ArabicOLED(Adafruit_SSD1306 &display);

  // Draws `utf8Text` right-to-left so that its rightmost glyph's right
  // edge lands at x = xRight, with the glyphs' top at y. Returns the x
  // coordinate of the left edge of the text actually drawn (useful for
  // knowing how much room a string took up).
  //
  // Does NOT call display.clearDisplay() or display.display() - call
  // those yourself so you can compose multiple draws before flushing.
  int16_t drawText(const char *utf8Text, int16_t xRight, int16_t y,
                    uint16_t color = SSD1306_WHITE);

  // Convenience wrapper around drawText(): draws right-aligned to the
  // display's right edge, on a fixed-height "line" numbered from 0 at
  // the top. Line height is ARABIC_GLYPH_HEIGHT + lineSpacing pixels.
  int16_t printLine(const char *utf8Text, uint8_t line,
                     uint16_t color = SSD1306_WHITE, uint8_t lineSpacing = 4);

  // Convenience wrapper: draws `utf8Text` horizontally centered on the
  // display at the given y.
  int16_t printCentered(const char *utf8Text, int16_t y,
                         uint16_t color = SSD1306_WHITE);

  // Measures the pixel width `utf8Text` would occupy when drawn, without
  // drawing anything. Useful for custom layout/centering.
  uint16_t textWidth(const char *utf8Text);

  // Height in pixels of one line of Arabic text drawn by this library
  // (all glyphs share this fixed height; width varies per letter).
  static uint8_t glyphHeight();

private:
  Adafruit_SSD1306 *_display;
};

#endif // ARABIC_OLED_H
