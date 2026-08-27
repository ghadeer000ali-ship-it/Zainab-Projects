/*
 * ArabicOLED.cpp
 * --------------
 * Implementation of the ArabicOLED class: UTF-8 decoding, Arabic
 * contextual-shaping (letter joining) and right-to-left rendering.
 *
 * HOW ARABIC JOINING WORKS (the short version)
 * -----------------------------------------------
 * Every Arabic letter falls into one of three "joining classes":
 *
 *   - Dual-joining:  can connect to a letter on BOTH sides
 *                     (e.g. bā', mīm, kāf ...). Has all 4 shapes.
 *   - Right-joining:  can only be connected to FROM a previous letter;
 *                     it never connects onward to the next one
 *                     (e.g. alef, dāl, rā', wāw, zāy, ة...). Has only
 *                     an isolated and a final shape.
 *   - Non-joining:    never connects either way (just hamza, ء). Has
 *                     only an isolated shape.
 *
 * ("Right" here is the traditional Unicode-shaping term and refers to
 * the letter's right-hand/leading side in the logical, not visual,
 * sense - it is the side that faces the *previous* character.)
 *
 * To pick a letter's shape we ask two questions:
 *   joinsPrev = does the PREVIOUS letter project a connection into us?
 *               (true only if the previous letter is dual-joining, and
 *               this letter accepts an incoming connection at all)
 *   joinsNext = do WE project a connection into the NEXT letter?
 *               (true only if we are dual-joining, and the next letter
 *               accepts an incoming connection)
 *
 *   joinsPrev && joinsNext  -> medial
 *   joinsPrev && !joinsNext -> final
 *   !joinsPrev && joinsNext -> initial
 *   neither                -> isolated
 *
 * Rather than storing bitmaps for the base letters and drawing them
 * differently per shape, we look each shape up directly by its Unicode
 * "Arabic Presentation Forms-B" codepoint (U+FE70-U+FEFC), which has a
 * dedicated codepoint for every (letter, shape) combination - e.g.
 * U+FEE3 is "MEEM INITIAL FORM". ArabicFont.h stores one bitmap per
 * such codepoint, pre-rendered offline. So "shaping" here just means:
 * for each letter, work out isolated/initial/medial/final, then look up
 * the matching presentation-form codepoint's bitmap.
 */

#include "ArabicOLED.h"
#include "ArabicFont.h"

// ---------------------------------------------------------------------
// Joining-class table for the Arabic base alphabet (U+0621-U+064A).
//
// For each base letter we store its joining class and the up-to-4
// presentation-form codepoints for its isolated/initial/medial/final
// shapes (0 = that shape does not exist for this letter).
// ---------------------------------------------------------------------
enum ArabicJoining : uint8_t {
  JOIN_NONE = 0,  // never connects (e.g. hamza)
  JOIN_RIGHT = 1, // connects only from the previous letter
  JOIN_DUAL = 2,  // connects on both sides
};

struct ArabicLetterInfo {
  uint16_t base;
  uint8_t joining;
  uint16_t isolated;
  uint16_t initial;
  uint16_t medial;
  uint16_t finalForm;
};

// Generated from Unicode's own presentation-form glyph names (see
// tools/generate_font.py) - not hand-typed, so this table's codepoints
// are guaranteed to match the naming in the Unicode standard.
static const ArabicLetterInfo ARABIC_LETTERS[] PROGMEM = {
  { 0x0621, JOIN_NONE,  0xFE80, 0,      0,      0      }, // HAMZA
  { 0x0622, JOIN_RIGHT, 0xFE81, 0,      0,      0xFE82 }, // ALEF MADDA
  { 0x0623, JOIN_RIGHT, 0xFE83, 0,      0,      0xFE84 }, // ALEF HAMZA ABOVE
  { 0x0624, JOIN_RIGHT, 0xFE85, 0,      0,      0xFE86 }, // WAW HAMZA
  { 0x0625, JOIN_RIGHT, 0xFE87, 0,      0,      0xFE88 }, // ALEF HAMZA BELOW
  { 0x0626, JOIN_DUAL,  0xFE89, 0xFE8B, 0xFE8C, 0xFE8A }, // YEH HAMZA
  { 0x0627, JOIN_RIGHT, 0xFE8D, 0,      0,      0xFE8E }, // ALEF
  { 0x0628, JOIN_DUAL,  0xFE8F, 0xFE91, 0xFE92, 0xFE90 }, // BEH
  { 0x0629, JOIN_RIGHT, 0xFE93, 0,      0,      0xFE94 }, // TEH MARBUTA
  { 0x062A, JOIN_DUAL,  0xFE95, 0xFE97, 0xFE98, 0xFE96 }, // TEH
  { 0x062B, JOIN_DUAL,  0xFE99, 0xFE9B, 0xFE9C, 0xFE9A }, // THEH
  { 0x062C, JOIN_DUAL,  0xFE9D, 0xFE9F, 0xFEA0, 0xFE9E }, // JEEM
  { 0x062D, JOIN_DUAL,  0xFEA1, 0xFEA3, 0xFEA4, 0xFEA2 }, // HAH
  { 0x062E, JOIN_DUAL,  0xFEA5, 0xFEA7, 0xFEA8, 0xFEA6 }, // KHAH
  { 0x062F, JOIN_RIGHT, 0xFEA9, 0,      0,      0xFEAA }, // DAL
  { 0x0630, JOIN_RIGHT, 0xFEAB, 0,      0,      0xFEAC }, // THAL
  { 0x0631, JOIN_RIGHT, 0xFEAD, 0,      0,      0xFEAE }, // REH
  { 0x0632, JOIN_RIGHT, 0xFEAF, 0,      0,      0xFEB0 }, // ZAIN
  { 0x0633, JOIN_DUAL,  0xFEB1, 0xFEB3, 0xFEB4, 0xFEB2 }, // SEEN
  { 0x0634, JOIN_DUAL,  0xFEB5, 0xFEB7, 0xFEB8, 0xFEB6 }, // SHEEN
  { 0x0635, JOIN_DUAL,  0xFEB9, 0xFEBB, 0xFEBC, 0xFEBA }, // SAD
  { 0x0636, JOIN_DUAL,  0xFEBD, 0xFEBF, 0xFEC0, 0xFEBE }, // DAD
  { 0x0637, JOIN_DUAL,  0xFEC1, 0xFEC3, 0xFEC4, 0xFEC2 }, // TAH
  { 0x0638, JOIN_DUAL,  0xFEC5, 0xFEC7, 0xFEC8, 0xFEC6 }, // ZAH
  { 0x0639, JOIN_DUAL,  0xFEC9, 0xFECB, 0xFECC, 0xFECA }, // AIN
  { 0x063A, JOIN_DUAL,  0xFECD, 0xFECF, 0xFED0, 0xFECE }, // GHAIN
  { 0x0641, JOIN_DUAL,  0xFED1, 0xFED3, 0xFED4, 0xFED2 }, // FEH
  { 0x0642, JOIN_DUAL,  0xFED5, 0xFED7, 0xFED8, 0xFED6 }, // QAF
  { 0x0643, JOIN_DUAL,  0xFED9, 0xFEDB, 0xFEDC, 0xFEDA }, // KAF
  { 0x0644, JOIN_DUAL,  0xFEDD, 0xFEDF, 0xFEE0, 0xFEDE }, // LAM
  { 0x0645, JOIN_DUAL,  0xFEE1, 0xFEE3, 0xFEE4, 0xFEE2 }, // MEEM
  { 0x0646, JOIN_DUAL,  0xFEE5, 0xFEE7, 0xFEE8, 0xFEE6 }, // NOON
  { 0x0647, JOIN_DUAL,  0xFEE9, 0xFEEB, 0xFEEC, 0xFEEA }, // HEH
  { 0x0648, JOIN_RIGHT, 0xFEED, 0,      0,      0xFEEE }, // WAW
  { 0x0649, JOIN_RIGHT, 0xFEEF, 0,      0,      0xFEF0 }, // ALEF MAKSURA
  { 0x064A, JOIN_DUAL,  0xFEF1, 0xFEF3, 0xFEF4, 0xFEF2 }, // YEH
};
static const uint8_t ARABIC_LETTER_COUNT =
    sizeof(ARABIC_LETTERS) / sizeof(ARABIC_LETTERS[0]);

// Optional "lam-alef" ligatures: a lam immediately followed by one of
// the four alef variants is conventionally drawn as a single joined
// glyph rather than two separate letters. This is a cosmetic nicety,
// not required for correct reading, but it is how real Arabic text
// looks and Unicode gives these ligatures their own codepoints too.
static const uint16_t LAM_CODEPOINT = 0x0644;
struct LamAlefInfo {
  uint16_t alefVariant;
  uint16_t isolated;
  uint16_t finalForm;
};
static const LamAlefInfo LAM_ALEF_TABLE[] PROGMEM = {
  { 0x0622, 0xFEF5, 0xFEF6 }, // LAM + ALEF WITH MADDA ABOVE
  { 0x0623, 0xFEF7, 0xFEF8 }, // LAM + ALEF WITH HAMZA ABOVE
  { 0x0625, 0xFEF9, 0xFEFA }, // LAM + ALEF WITH HAMZA BELOW
  { 0x0627, 0xFEFB, 0xFEFC }, // LAM + ALEF
};
static const uint8_t LAM_ALEF_COUNT =
    sizeof(LAM_ALEF_TABLE) / sizeof(LAM_ALEF_TABLE[0]);

// Space between glyphs isn't stored as a bitmap; this is how wide a
// space advances the cursor.
static const uint8_t ARABIC_SPACE_WIDTH = 6;

// Maximum number of Unicode codepoints handled per drawText()/
// textWidth() call. Generous for OLED-sized UI strings while keeping
// everything on the stack (no heap allocation).
static const size_t MAX_CODEPOINTS = 160;

// -----------------------------------------------------------------
// UTF-8 decoding
// -----------------------------------------------------------------
static size_t utf8Decode(const char *s, uint32_t *out, size_t maxOut) {
  size_t n = 0;
  while (*s != '\0' && n < maxOut) {
    uint8_t c = (uint8_t)*s++;
    uint32_t cp;
    int extra;
    if ((c & 0x80) == 0x00) { cp = c; extra = 0; }
    else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; extra = 1; }
    else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; extra = 2; }
    else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; extra = 3; }
    else { continue; } // stray continuation byte / invalid lead byte: skip

    bool valid = true;
    for (int k = 0; k < extra; k++) {
      uint8_t cc = (uint8_t)*s;
      if (cc == 0 || (cc & 0xC0) != 0x80) { valid = false; break; }
      cp = (cp << 6) | (cc & 0x3F);
      s++;
    }
    if (valid) out[n++] = cp;
  }
  return n;
}

// -----------------------------------------------------------------
// Joining-table lookups
// -----------------------------------------------------------------
static const ArabicLetterInfo *findLetterInfo(uint32_t cp) {
  for (uint8_t i = 0; i < ARABIC_LETTER_COUNT; i++) {
    if (ARABIC_LETTERS[i].base == cp) return &ARABIC_LETTERS[i];
  }
  return nullptr;
}

// Can `cp` project a connection forward into the following letter?
static bool canJoinNext(uint32_t cp) {
  const ArabicLetterInfo *info = findLetterInfo(cp);
  return info != nullptr && info->joining == JOIN_DUAL;
}

// Can `cp` accept an incoming connection from the previous letter?
static bool canJoinPrev(uint32_t cp) {
  const ArabicLetterInfo *info = findLetterInfo(cp);
  return info != nullptr &&
         (info->joining == JOIN_DUAL || info->joining == JOIN_RIGHT);
}

// -----------------------------------------------------------------
// Shaping: logical codepoints in -> display (glyph) codepoints out.
// Output length is <= input length (lam-alef ligatures merge 2 -> 1).
// Non-Arabic codepoints (spaces, digits, Latin, punctuation) pass
// through unchanged, acting as "joining breaks" for their neighbours -
// exactly like Arabic's own non-joining letters do.
// -----------------------------------------------------------------
static size_t shapeArabic(const uint32_t *cps, size_t n, uint32_t *out) {
  size_t oi = 0;
  size_t i = 0;
  while (i < n) {
    uint32_t cp = cps[i];

    // Lam-alef ligature: consume this lam + the next alef-variant as
    // a single glyph.
    if (cp == LAM_CODEPOINT && i + 1 < n) {
      const LamAlefInfo *lig = nullptr;
      for (uint8_t k = 0; k < LAM_ALEF_COUNT; k++) {
        if (LAM_ALEF_TABLE[k].alefVariant == cps[i + 1]) {
          lig = &LAM_ALEF_TABLE[k];
          break;
        }
      }
      if (lig != nullptr) {
        bool joinsPrev = (i > 0) && canJoinPrev(LAM_CODEPOINT) &&
                          canJoinNext(cps[i - 1]);
        out[oi++] = joinsPrev ? lig->finalForm : lig->isolated;
        i += 2;
        continue;
      }
    }

    const ArabicLetterInfo *info = findLetterInfo(cp);
    if (info == nullptr) {
      // Not an Arabic letter (space, digit, Latin letter, punctuation
      // such as the Arabic question mark...) - pass through unchanged.
      out[oi++] = cp;
      i++;
      continue;
    }

    bool joinsPrev =
        (i > 0) && canJoinPrev(cp) && canJoinNext(cps[i - 1]);
    bool joinsNext =
        (i + 1 < n) && canJoinNext(cp) && canJoinPrev(cps[i + 1]);

    uint16_t glyphCp;
    if (joinsPrev && joinsNext) {
      glyphCp = info->medial ? info->medial : info->isolated;
    } else if (joinsPrev) {
      glyphCp = info->finalForm ? info->finalForm : info->isolated;
    } else if (joinsNext) {
      glyphCp = info->initial ? info->initial : info->isolated;
    } else {
      glyphCp = info->isolated;
    }
    out[oi++] = glyphCp;
    i++;
  }
  return oi;
}

// -----------------------------------------------------------------
// Low-level glyph blit: draws one packed 1-bpp glyph bitmap.
// -----------------------------------------------------------------
static void drawGlyphBitmap(Adafruit_SSD1306 *display, int16_t x, int16_t y,
                             const ArabicGlyph *g, uint16_t color) {
  uint8_t bytesPerRow = (g->width + 7) / 8;
  for (uint8_t row = 0; row < ARABIC_GLYPH_HEIGHT; row++) {
    const uint8_t *rowBytes = g->bitmap + (size_t)row * bytesPerRow;
    for (uint8_t col = 0; col < g->width; col++) {
      uint8_t byte = rowBytes[col / 8];
      if (byte & (0x80 >> (col % 8))) {
        display->drawPixel(x + col, y + row, color);
      }
    }
  }
}

// -----------------------------------------------------------------
// Shared layout walk used by both drawText() (draw = true) and
// textWidth() (draw = false). Runs right-to-left: each glyph/run's
// width is subtracted from the cursor BEFORE it is drawn, since the
// first (rightmost) element in logical order sits at the right edge.
// -----------------------------------------------------------------
static int16_t layoutText(Adafruit_SSD1306 *display, const char *utf8Text,
                           int16_t xRight, int16_t y, uint16_t color,
                           bool draw) {
  uint32_t cps[MAX_CODEPOINTS];
  size_t n = utf8Decode(utf8Text, cps, MAX_CODEPOINTS);

  uint32_t shaped[MAX_CODEPOINTS];
  size_t m = shapeArabic(cps, n, shaped);

  int16_t cursorX = xRight;
  size_t i = 0;
  while (i < m) {
    uint32_t cp = shaped[i];
    const ArabicGlyph *g = arabicFindGlyph((uint16_t)cp);

    if (g != nullptr || cp == 0x20) {
      // A single Arabic glyph (or a plain space).
      uint8_t w = (g != nullptr) ? g->width : ARABIC_SPACE_WIDTH;
      cursorX -= w;
      if (draw && g != nullptr) {
        drawGlyphBitmap(display, cursorX, y, g, color);
      }
      i++;
    } else {
      // A run of non-Arabic characters (Latin letters, digits, other
      // punctuation): gather them and draw as one left-to-right block
      // using Adafruit_GFX's built-in font, positioned at the correct
      // point in the right-to-left flow.
      char run[48];
      size_t runLen = 0;
      while (i < m) {
        uint32_t c2 = shaped[i];
        const ArabicGlyph *g2 = arabicFindGlyph((uint16_t)c2);
        if (g2 != nullptr || c2 == 0x20) break;
        if (c2 < 128 && runLen < sizeof(run) - 1) {
          run[runLen++] = (char)c2;
        }
        i++;
      }
      run[runLen] = '\0';

      int16_t bx, by;
      uint16_t bw, bh;
      display->setTextSize(1);
      display->getTextBounds(run, 0, 0, &bx, &by, &bw, &bh);
      cursorX -= bw;
      if (draw) {
        display->setTextColor(color);
        display->setCursor(cursorX, y + (ARABIC_GLYPH_HEIGHT - 8) / 2);
        display->print(run);
      }
    }
  }
  return cursorX;
}

// -----------------------------------------------------------------
// Public API
// -----------------------------------------------------------------
ArabicOLED::ArabicOLED(Adafruit_SSD1306 &display) : _display(&display) {}

int16_t ArabicOLED::drawText(const char *utf8Text, int16_t xRight, int16_t y,
                              uint16_t color) {
  return layoutText(_display, utf8Text, xRight, y, color, true);
}

uint16_t ArabicOLED::textWidth(const char *utf8Text) {
  int16_t xRight = 0;
  int16_t leftEdge = layoutText(_display, utf8Text, xRight, 0, 0, false);
  return (uint16_t)(xRight - leftEdge);
}

int16_t ArabicOLED::printLine(const char *utf8Text, uint8_t line,
                               uint16_t color, uint8_t lineSpacing) {
  int16_t xRight = _display->width() - 1;
  int16_t y = line * (ARABIC_GLYPH_HEIGHT + lineSpacing);
  return drawText(utf8Text, xRight, y, color);
}

int16_t ArabicOLED::printCentered(const char *utf8Text, int16_t y,
                                   uint16_t color) {
  uint16_t w = textWidth(utf8Text);
  int16_t xRight = (_display->width() + (int16_t)w) / 2;
  return drawText(utf8Text, xRight, y, color);
}

uint8_t ArabicOLED::glyphHeight() { return ARABIC_GLYPH_HEIGHT; }
