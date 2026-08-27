/*
 * RoomView.h
 * ----------
 * Draws the static parts of Zeito's room: the window, the bed, the
 * plant and the floor. Pure drawing, no state and no timers - call
 * draw() once per frame after clearDisplay(), before drawing Zeito on
 * top of it.
 */
#ifndef ROOM_VIEW_H
#define ROOM_VIEW_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class RoomView {
public:
  explicit RoomView(Adafruit_SSD1306 &display) : _display(&display) {}

  // Draws the whole room background (window, bed, plant, floor).
  void draw();

private:
  Adafruit_SSD1306 *_display;

  void drawFloor();
  void drawWindow();
  void drawBed();
  void drawPlant();
};

#endif // ROOM_VIEW_H
