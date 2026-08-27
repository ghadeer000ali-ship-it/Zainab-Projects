/*
 * ZeitoView.h
 * -----------
 * The DRAWING layer for the character. Turns a ZeitoViewState
 * (produced by ZeitoBrain) into pixels. No timers, no randomness, no
 * notion of "what happens next" beyond tiny cosmetic bobbing (water
 * drops, Zzz) that carries no behavioral meaning of its own - the same
 * split used by RoomView (static scenery) and ZeitoBrain (behavior).
 */
#ifndef ZEITO_VIEW_H
#define ZEITO_VIEW_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HomeTypes.h"

class ZeitoView {
public:
  explicit ZeitoView(Adafruit_SSD1306 &display) : _display(&display) {}

  // Draws Zeito (and any active watering-can / Zzz / bubble overlay)
  // for the given state. Does not clear or flush the display.
  void draw(const ZeitoViewState &state);

private:
  Adafruit_SSD1306 *_display;

  void drawCharacter(const ZeitoViewState &state);
  void drawWateringCan(int16_t charX, int16_t charTopY, bool facingRight);
  void drawZzz(int16_t charX, int16_t charTopY, uint8_t count); // count: 1=Z, 2=ZZ, 3=ZZZ
  void drawBubble(int16_t charX, int16_t charTopY, BubbleIcon icon);

  void drawSprite(int16_t centerX, int16_t topY, const uint8_t *rows, bool mirror);
  void drawIcon(int16_t centerX, int16_t centerY, const uint8_t *rows);
  void drawZChar(int16_t cx, int16_t cy, int16_t scale);

  static uint8_t reverseByte(uint8_t b);
};

#endif // ZEITO_VIEW_H
