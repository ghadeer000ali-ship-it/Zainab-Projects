/*
 * ZeitoSprites.h
 * --------------
 * Raw pixel-art bitmap data - one byte per row, 8 pixels wide (bit 7 =
 * leftmost pixel), stored in PROGMEM. This file is pure data: no logic,
 * no drawing code. See ZeitoView.cpp for how these are rendered
 * (including horizontal mirroring for walking left).
 *
 * Character frames are HomeConfig::SPRITE_W x HomeConfig::SPRITE_H.
 * Bubble icons are HomeConfig::ICON_W x HomeConfig::ICON_H.
 */
#ifndef ZEITO_SPRITES_H
#define ZEITO_SPRITES_H

#include <Arduino.h>
#include "HomeConfig.h"

// ===========================================================
// Zeito - character frames
// ===========================================================
static const uint8_t ZEITO_STAND[HomeConfig::SPRITE_H] PROGMEM = {
  0x3C, 0x7E, 0xFF, 0xBD, 0xA5, 0xBD, 0xBD, 0x3C, 0x7E, 0xFF, 0xDB, 0xDB
};
static const uint8_t ZEITO_BLINK[HomeConfig::SPRITE_H] PROGMEM = {
  0x3C, 0x7E, 0xFF, 0xBD, 0x81, 0xBD, 0xBD, 0x3C, 0x7E, 0xFF, 0xDB, 0xDB
};
static const uint8_t ZEITO_SMILE[HomeConfig::SPRITE_H] PROGMEM = {
  0x3C, 0x7E, 0xFF, 0xBD, 0xA5, 0xA5, 0xBD, 0x7C, 0x7E, 0xFF, 0xDB, 0xDB
};
static const uint8_t ZEITO_WALK1[HomeConfig::SPRITE_H] PROGMEM = {
  0x3C, 0x7E, 0xFF, 0xBD, 0xA5, 0xBD, 0xBD, 0x3C, 0x7E, 0xFF, 0x6C, 0xD8
};
static const uint8_t ZEITO_WALK2[HomeConfig::SPRITE_H] PROGMEM = {
  0x3C, 0x7E, 0xFF, 0xBD, 0xA5, 0xBD, 0xBD, 0x3C, 0x7E, 0xFF, 0x36, 0x1B
};
static const uint8_t ZEITO_SLEEP[HomeConfig::SPRITE_H] PROGMEM = {
  0x00, 0x00, 0x00, 0x3C, 0x5A, 0xFF, 0x81, 0x81, 0x81, 0xFF, 0x00, 0x00
};

// ===========================================================
// Dream / speech bubble icons (7x7)
// ===========================================================
static const uint8_t ICON_STAR[HomeConfig::ICON_H] PROGMEM = { 0x10, 0x10, 0x92, 0x54, 0xEE, 0x44, 0x82 };
static const uint8_t ICON_HEART[HomeConfig::ICON_H] PROGMEM = { 0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10 };
static const uint8_t ICON_CAT[HomeConfig::ICON_H] PROGMEM = { 0x82, 0xC6, 0xFE, 0xAA, 0x92, 0xBA, 0x44 };
static const uint8_t ICON_BEAR[HomeConfig::ICON_H] PROGMEM = { 0x82, 0x7C, 0xFE, 0xAA, 0x92, 0xBA, 0x7C };
static const uint8_t ICON_FLOWER[HomeConfig::ICON_H] PROGMEM = { 0x54, 0xBA, 0x7C, 0xBA, 0x54, 0x10, 0x38 };
static const uint8_t ICON_ROCKET[HomeConfig::ICON_H] PROGMEM = { 0x10, 0x38, 0x38, 0x7C, 0xFE, 0x54, 0x92 };
static const uint8_t ICON_PIZZA[HomeConfig::ICON_H] PROGMEM = { 0xFE, 0x7C, 0x38, 0x28, 0x44, 0x82, 0xFE };
static const uint8_t ICON_NOTE[HomeConfig::ICON_H] PROGMEM = { 0x18, 0x18, 0x18, 0x3C, 0x6C, 0xEC, 0xCC };

#endif // ZEITO_SPRITES_H
