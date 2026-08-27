#include "MenuView.h"
#include "HomeConfig.h"

using namespace HomeConfig;

const char *const MenuView::ITEM_LABELS[MenuView::ITEM_COUNT] = { "Talk", "Sleep", "Play", "Feed" };
const MenuAction MenuView::ITEM_ACTIONS[MenuView::ITEM_COUNT] = {
  MenuAction::TALK, MenuAction::SLEEP, MenuAction::PLAY, MenuAction::FEED
};

void MenuView::open(unsigned long now) {
  _open = true;
  _selectedIndex = 0;
  _lastActivityAt = now;
}

void MenuView::close() {
  _open = false;
}

void MenuView::update(unsigned long now) {
  if (!_open) return;
  if (now - _lastActivityAt >= MENU_AUTO_CLOSE_MS) {
    close();
  }
}

void MenuView::navigateUp(unsigned long now) {
  if (!_open) return;
  _selectedIndex = (_selectedIndex == 0) ? (ITEM_COUNT - 1) : (_selectedIndex - 1);
  _lastActivityAt = now;
}

void MenuView::navigateDown(unsigned long now) {
  if (!_open) return;
  _selectedIndex = (_selectedIndex + 1) % ITEM_COUNT;
  _lastActivityAt = now;
}

MenuAction MenuView::confirmSelection(unsigned long now) {
  if (!_open) return MenuAction::NONE;
  _lastActivityAt = now;
  return ITEM_ACTIONS[_selectedIndex];
}

void MenuView::draw() const {
  if (!_open) return;

  const int16_t boxX = 28, boxY = 6, boxW = 72, boxH = 52;
  const int16_t rowH = 9;
  const int16_t firstRowY = boxY + 14;

  _display->fillRect(boxX, boxY, boxW, boxH, SSD1306_BLACK);
  _display->drawRoundRect(boxX, boxY, boxW, boxH, 3, SSD1306_WHITE);

  _display->setTextSize(1);
  _display->setTextColor(SSD1306_WHITE);
  _display->setCursor(boxX + 6, boxY + 4);
  _display->print(F("Menu"));
  _display->drawFastHLine(boxX + 4, boxY + 12, boxW - 8, SSD1306_WHITE);

  for (uint8_t i = 0; i < ITEM_COUNT; i++) {
    int16_t rowY = firstRowY + i * rowH;
    bool selected = (i == _selectedIndex);

    if (selected) {
      _display->fillRect(boxX + 3, rowY - 1, boxW - 6, rowH, SSD1306_WHITE);
      _display->setTextColor(SSD1306_BLACK);
    } else {
      _display->setTextColor(SSD1306_WHITE);
    }
    _display->setCursor(boxX + 8, rowY);
    _display->print(ITEM_LABELS[i]);
  }
}
