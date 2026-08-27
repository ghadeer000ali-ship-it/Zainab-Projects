#include "ZeitoView.h"
#include "HomeConfig.h"
#include "ZeitoSprites.h"
#include <math.h>

using namespace HomeConfig;

// ===========================================================
// render() - top-level dispatch
// ===========================================================
void ZeitoView::draw(const ZeitoViewState &state) {
  drawCharacter(state);

  if (state.showWateringCan) {
    drawWateringCan(state.x, CHAR_TOP_Y, state.facingRight);
  }
  if (state.zzzCount > 0) {
    drawZzz(state.x, CHAR_TOP_Y, state.zzzCount);
  }
  if (state.showBubble) {
    drawBubble(state.x, CHAR_TOP_Y, state.bubbleIcon);
  }
}

// ===========================================================
// Character
// ===========================================================
void ZeitoView::drawCharacter(const ZeitoViewState &state) {
  const uint8_t *frame;

  if (state.lying) {
    frame = ZEITO_SLEEP;
  } else if (state.eyesClosed) {
    frame = ZEITO_BLINK;
  } else if (state.smiling) {
    frame = ZEITO_SMILE;
  } else if (state.isWalking) {
    frame = state.stepParity ? ZEITO_WALK1 : ZEITO_WALK2;
  } else {
    frame = ZEITO_STAND;
  }

  bool mirror = !state.facingRight; // sprite is authored facing right
  drawSprite(state.x, CHAR_TOP_Y, frame, mirror);
}

// ===========================================================
// Watering can + falling drops (purely cosmetic bob, no behavior)
// ===========================================================
void ZeitoView::drawWateringCan(int16_t charX, int16_t charTopY, bool facingRight) {
  int8_t side = facingRight ? 1 : -1;
  int16_t canX = charX + side * (SPRITE_W / 2 + 4);
  int16_t canY = charTopY + 6;

  _display->drawRect(canX - 2, canY, 5, 4, SSD1306_WHITE);
  _display->drawFastHLine(canX + (side > 0 ? 2 : -5), canY + 1, 3, SSD1306_WHITE); // spout

  // A drop that falls and resets, timed purely from millis() - cosmetic only.
  int16_t dropX = canX + side * 4;
  int16_t dropOffset = (int16_t)(millis() % 600) / 100; // 0..5
  _display->drawPixel(dropX, canY + 4 + dropOffset, SSD1306_WHITE);
}

// ===========================================================
// Sleeping Zzz (gentle upward drift, same trick as a bobbing icon).
// ZeitoBrain decides HOW MANY Z's to show right now (the progressive
// Z -> ZZ -> ZZZ reveal while falling asleep is a meaningful decision,
// not cosmetic); this just draws that many, biggest-and-closest first.
// ===========================================================
void ZeitoView::drawZzz(int16_t charX, int16_t charTopY, uint8_t count) {
  if (count > 3) count = 3;
  static const int16_t dx[3] = {0, 5, 9};
  static const int16_t dy[3] = {0, -6, -11};
  static const int16_t sz[3] = {3, 2, 1};

  float bob = sinf(millis() / 400.0f) * 1.0f;
  int16_t x = charX + SPRITE_W / 2 + 2;
  int16_t y = charTopY - 2 + (int16_t)lroundf(bob);

  for (uint8_t i = 0; i < count; i++) {
    drawZChar(x + dx[i], y + dy[i], sz[i]);
  }
}

// ===========================================================
// Dream / speech bubble
// ===========================================================
void ZeitoView::drawBubble(int16_t charX, int16_t charTopY, BubbleIcon icon) {
  const uint8_t BOX_W = 18, BOX_H = 15;
  int16_t bx = charX - BOX_W / 2;
  if (bx < 0) bx = 0;
  if (bx > SCREEN_WIDTH - BOX_W) bx = SCREEN_WIDTH - BOX_W;
  int16_t by = charTopY - BOX_H - 4;
  if (by < 0) by = 0;

  _display->fillRect(bx, by, BOX_W, BOX_H, SSD1306_BLACK);
  _display->drawRoundRect(bx, by, BOX_W, BOX_H, 3, SSD1306_WHITE);
  // Little pointer tail toward Zeito's head.
  _display->drawPixel(charX, by + BOX_H, SSD1306_WHITE);
  _display->drawPixel(charX - 1, by + BOX_H + 1, SSD1306_WHITE);
  _display->drawPixel(charX + 1, by + BOX_H + 1, SSD1306_WHITE);

  const uint8_t *rows;
  switch (icon) {
    case BubbleIcon::STAR:   rows = ICON_STAR;   break;
    case BubbleIcon::HEART:  rows = ICON_HEART;  break;
    case BubbleIcon::CAT:    rows = ICON_CAT;    break;
    case BubbleIcon::BEAR:   rows = ICON_BEAR;   break;
    case BubbleIcon::FLOWER: rows = ICON_FLOWER; break;
    case BubbleIcon::ROCKET: rows = ICON_ROCKET; break;
    case BubbleIcon::PIZZA:  rows = ICON_PIZZA;  break;
    case BubbleIcon::NOTE:   rows = ICON_NOTE;   break;
    default: return;
  }
  drawIcon(bx + BOX_W / 2, by + BOX_H / 2, rows);
}

// ===========================================================
// Low-level primitives
// ===========================================================
void ZeitoView::drawSprite(int16_t centerX, int16_t topY, const uint8_t *rows, bool mirror) {
  int16_t x0 = centerX - SPRITE_W / 2;
  for (uint8_t row = 0; row < SPRITE_H; row++) {
    uint8_t bits = pgm_read_byte(&rows[row]);
    if (mirror) bits = reverseByte(bits);
    for (uint8_t col = 0; col < SPRITE_W; col++) {
      if (bits & (0x80 >> col)) {
        _display->drawPixel(x0 + col, topY + row, SSD1306_WHITE);
      }
    }
  }
}

void ZeitoView::drawIcon(int16_t centerX, int16_t centerY, const uint8_t *rows) {
  int16_t x0 = centerX - ICON_W / 2;
  int16_t y0 = centerY - ICON_H / 2;
  for (uint8_t row = 0; row < ICON_H; row++) {
    uint8_t bits = pgm_read_byte(&rows[row]);
    for (uint8_t col = 0; col < ICON_W; col++) {
      if (bits & (0x80 >> col)) {
        _display->drawPixel(x0 + col, y0 + row, SSD1306_WHITE);
      }
    }
  }
}

// One "Z" glyph, three strokes - cheaper than pulling in a text font
// just for this, and easy to scale down for the trailing Z's.
void ZeitoView::drawZChar(int16_t cx, int16_t cy, int16_t s) {
  if (s < 1) s = 1;
  _display->drawFastHLine(cx - s, cy - s, 2 * s + 1, SSD1306_WHITE);
  _display->drawLine(cx + s, cy - s, cx - s, cy + s, SSD1306_WHITE);
  _display->drawFastHLine(cx - s, cy + s, 2 * s + 1, SSD1306_WHITE);
}

uint8_t ZeitoView::reverseByte(uint8_t b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}
