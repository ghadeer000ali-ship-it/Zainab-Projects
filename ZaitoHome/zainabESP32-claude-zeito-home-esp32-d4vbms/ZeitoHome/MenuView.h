/*
 * MenuView.h
 * ----------
 * The joystick never drives Zeito directly - moving it just opens this
 * small popup with four choices: Talk, Sleep, Play, Feed. This class
 * owns both the tiny bit of UI state (open/closed, selected row, an
 * auto-close idle timer) and its own drawing, since a popup menu is a
 * self-contained feature independent of the room/character views.
 *
 * The top-level sketch/facade wires Joystick events into this class's
 * navigate/confirm methods, and turns confirmSelection()'s result into
 * the matching ZeitoBrain::requestXxx() call - MenuView itself knows
 * nothing about ZeitoBrain.
 */
#ifndef MENU_VIEW_H
#define MENU_VIEW_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

enum class MenuAction : uint8_t { NONE, TALK, SLEEP, PLAY, FEED };

class MenuView {
public:
  explicit MenuView(Adafruit_SSD1306 &display) : _display(&display) {}

  bool isOpen() const { return _open; }

  void open(unsigned long now);
  void close();

  // Call every loop() iteration while open() might be true - closes the
  // menu on its own after HomeConfig::MENU_AUTO_CLOSE_MS of no input.
  void update(unsigned long now);

  void navigateUp(unsigned long now);
  void navigateDown(unsigned long now);

  // Returns which item is currently highlighted, without closing the menu.
  MenuAction confirmSelection(unsigned long now);

  void draw() const;

private:
  Adafruit_SSD1306 *_display;
  bool _open = false;
  uint8_t _selectedIndex = 0;
  unsigned long _lastActivityAt = 0;

  static const uint8_t ITEM_COUNT = 4;
  static const char *const ITEM_LABELS[ITEM_COUNT];
  static const MenuAction ITEM_ACTIONS[ITEM_COUNT];
};

#endif // MENU_VIEW_H
