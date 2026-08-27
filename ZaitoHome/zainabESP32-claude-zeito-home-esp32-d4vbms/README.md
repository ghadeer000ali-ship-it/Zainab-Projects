# ArabicOLED

An Arduino library that draws right-to-left Arabic text - with automatic
letter joining (isolated/initial/medial/final forms) - on 128x64 I2C
SSD1306 OLED displays, built on top of `Adafruit_GFX` and
`Adafruit_SSD1306`. Targets ESP32.

## Features

- Accepts plain **UTF-8** Arabic text (just type Arabic in your `.ino`
  file - the Arduino IDE and PlatformIO both save source files as UTF-8).
- **Automatic contextual shaping**: works out whether each letter needs
  its isolated, initial, medial or final form from its neighbours, the
  same way a word processor would. You never pick letter shapes
  yourself.
- Renders **right-to-left**.
- Optional lam-alef ligature ("لا") handling.
- Falls back to `Adafruit_GFX`'s built-in font for any non-Arabic
  characters (digits, Latin text) so mixed content doesn't disappear.
- Self-contained bitmap font (`src/ArabicFont.h`) - no external font
  files or filesystem access needed at runtime.

## Files

| File                    | Purpose                                                             |
|--------------------------|----------------------------------------------------------------------|
| `src/ArabicOLED.h`       | Public `ArabicOLED` class declaration.                              |
| `src/ArabicOLED.cpp`     | UTF-8 decoding, Arabic shaping/joining logic, right-to-left drawing.|
| `src/ArabicFont.h`       | Bitmap glyph data for every Arabic letter shape (127 glyphs).       |
| `tools/generate_font.py` | Optional offline script used to (re)generate `ArabicFont.h`.        |
| `examples/ArabicOLED_Demo` | Example sketch: prints مرحبا / زيتونه / كيف حالك؟                |

## Installation

1. Copy this repository into your Arduino `libraries/` folder (or add it
   as a library in PlatformIO), keeping the `src/` layout.
2. Install the dependencies via Library Manager:
   - **Adafruit GFX Library**
   - **Adafruit SSD1306**
3. Open `examples/ArabicOLED_Demo/ArabicOLED_Demo.ino`, select your ESP32
   board, and upload.

## Wiring (ESP32 default I2C pins)

| OLED pin | ESP32 pin |
|----------|-----------|
| SDA      | GPIO 21   |
| SCL      | GPIO 22   |
| VCC      | 3V3       |
| GND      | GND       |

## Quick start

```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArabicOLED.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
ArabicOLED arabic(display);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  arabic.printCentered("مرحبا", 20);   // horizontally centered
  arabic.printLine("زيتونه", 1);       // right-aligned "line" 1

  display.display();
}

void loop() {}
```

## API

```cpp
ArabicOLED arabic(display); // display must already be display.begin()'d

// Draw text right-to-left so its right edge lands at xRight, top at y.
// Returns the x of the left edge of what was drawn.
int16_t arabic.drawText(const char *utf8Text, int16_t xRight, int16_t y,
                         uint16_t color = SSD1306_WHITE);

// Right-aligned to the screen edge, on a fixed-height numbered line.
int16_t arabic.printLine(const char *utf8Text, uint8_t line,
                          uint16_t color = SSD1306_WHITE,
                          uint8_t lineSpacing = 4);

// Horizontally centered on the screen at the given y.
int16_t arabic.printCentered(const char *utf8Text, int16_t y,
                              uint16_t color = SSD1306_WHITE);

// Pixel width utf8Text would take up, without drawing it.
uint16_t arabic.textWidth(const char *utf8Text);

// Fixed pixel height of one line of Arabic glyphs (24).
uint8_t ArabicOLED::glyphHeight();
```

`drawText`/`printLine`/`printCentered` do not call
`display.clearDisplay()` or `display.display()` themselves, so you can
compose several draws before flushing to the screen.

## How the shaping works

Arabic letters fall into three joining classes:

- **Dual-joining** letters (most of the alphabet) connect to a letter on
  *both* sides and have 4 shapes: isolated, initial, medial, final.
- **Right-joining** letters (`ا د ذ ر ز و` and a few others) only accept
  an incoming connection from the previous letter; they never connect
  onward. They have only 2 shapes: isolated and final.
- **Non-joining**: just hamza (`ء`), which never connects and has only
  one shape.

For each letter, `ArabicOLED.cpp` checks whether the previous letter
projects a connection into it and whether it projects one into the next
letter, then picks isolated/initial/medial/final accordingly - see the
comment block at the top of `shapeArabic()` in `src/ArabicOLED.cpp` for
the full truth table.

Each resulting shape is looked up directly by its Unicode "Arabic
Presentation Forms-B" codepoint (e.g. U+FEE3 = "MEEM INITIAL FORM") in
`src/ArabicFont.h`, which stores one pre-rendered bitmap per shape.

## Limitations

- No Arabic diacritics (harakat/tashkeel) - base letters only.
- Not a full Unicode Bidirectional Algorithm implementation: a single
  embedded Latin word or number inside an Arabic sentence renders
  correctly, but multiple separate Latin words mixed into Arabic text
  will still be ordered right-to-left relative to each other.
- Text buffers are stack-allocated with a generous but fixed limit
  (`MAX_CODEPOINTS = 160` in `ArabicOLED.cpp`); longer strings are
  truncated.

## Regenerating the font

The bitmap font was rasterized offline from the FreeSerif font (GNU
FreeFont project), which has native glyphs for every Arabic
Presentation-Forms-B codepoint. You don't need to do this to use the
library, but if you want a different size or font, see
`tools/generate_font.py`.
