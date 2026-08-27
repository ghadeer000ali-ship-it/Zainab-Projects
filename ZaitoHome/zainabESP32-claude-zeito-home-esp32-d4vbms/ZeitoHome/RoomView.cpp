#include "RoomView.h"
#include "HomeConfig.h"

using namespace HomeConfig;

void RoomView::draw() {
  drawWindow();
  drawPlant();
  drawBed();
  drawFloor();
}

void RoomView::drawFloor() {
  _display->drawFastHLine(0, FLOOR_Y, SCREEN_WIDTH, SSD1306_WHITE);
}

void RoomView::drawWindow() {
  _display->drawRect(WINDOW_X, WINDOW_Y, WINDOW_W, WINDOW_H, SSD1306_WHITE);
  // Cross bars splitting the window into four panes.
  _display->drawFastVLine(WINDOW_X + WINDOW_W / 2, WINDOW_Y, WINDOW_H, SSD1306_WHITE);
  _display->drawFastHLine(WINDOW_X, WINDOW_Y + WINDOW_H / 2, WINDOW_W, SSD1306_WHITE);
  // Sill, a little wider than the frame.
  _display->drawFastHLine(WINDOW_X - 2, WINDOW_Y + WINDOW_H, WINDOW_W + 4, SSD1306_WHITE);
}

void RoomView::drawBed() {
  _display->drawRect(BED_X, BED_Y, BED_W, BED_H, SSD1306_WHITE);
  // Pillow.
  _display->drawRoundRect(BED_PILLOW_X, BED_PILLOW_Y, BED_PILLOW_W, BED_PILLOW_H, 2, SSD1306_WHITE);
  // Blanket fold lines across the rest of the bed.
  int16_t blanketTop = BED_PILLOW_Y + BED_PILLOW_H + 4;
  for (int16_t y = blanketTop; y < BED_Y + BED_H - 2; y += 4) {
    _display->drawFastHLine(BED_X + 2, y, BED_W - 4, SSD1306_WHITE);
  }
  // Headboard / footboard posts.
  _display->drawFastVLine(BED_X, BED_Y - 3, BED_H + 3, SSD1306_WHITE);
  _display->drawFastVLine(BED_X + BED_W - 1, BED_Y - 3, BED_H + 3, SSD1306_WHITE);
}

void RoomView::drawPlant() {
  // Pot: a small trapezoid (narrower at the base).
  int16_t potTopY = PLANT_POT_Y;
  int16_t potBottomY = PLANT_POT_Y + PLANT_POT_H;
  _display->drawFastHLine(PLANT_POT_X, potTopY, PLANT_POT_W, SSD1306_WHITE);
  _display->drawLine(PLANT_POT_X, potTopY, PLANT_POT_X + 2, potBottomY, SSD1306_WHITE);
  _display->drawLine(PLANT_POT_X + PLANT_POT_W, potTopY, PLANT_POT_X + PLANT_POT_W - 2, potBottomY, SSD1306_WHITE);
  _display->drawFastHLine(PLANT_POT_X + 2, potBottomY, PLANT_POT_W - 4, SSD1306_WHITE);

  // Leaves: three simple curved strokes rising out of the pot.
  int16_t cx = PLANT_LEAVES_CX;
  _display->drawLine(cx, potTopY, cx - 5, PLANT_LEAVES_TOP_Y + 4, SSD1306_WHITE);
  _display->drawLine(cx - 5, PLANT_LEAVES_TOP_Y + 4, cx - 3, PLANT_LEAVES_TOP_Y, SSD1306_WHITE);
  _display->drawLine(cx, potTopY, cx, PLANT_LEAVES_TOP_Y, SSD1306_WHITE);
  _display->drawLine(cx, potTopY, cx + 5, PLANT_LEAVES_TOP_Y + 4, SSD1306_WHITE);
  _display->drawLine(cx + 5, PLANT_LEAVES_TOP_Y + 4, cx + 3, PLANT_LEAVES_TOP_Y, SSD1306_WHITE);
}
