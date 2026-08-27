/*
 * PetConfig.h
 * -----------
 * Central place for every pin assignment, display setting, geometry
 * constant and timing constant used by the Zaito virtual-pet project.
 *
 * WHY THIS FILE EXISTS
 * ----------------------
 * Nothing here is "logic" - it's all named, tunable numbers. If you want
 * to make Zaito's eyes bigger, blink less often, or wire the display to
 * different pins, you should only ever need to change this file, never
 * hunt through PetBrain.cpp / PetFace.cpp for a magic number.
 *
 * This is also the natural place to add new pin definitions when you
 * wire up buttons, a touch sensor, etc. later (see the "FUTURE
 * PERIPHERALS" section at the bottom).
 */
#ifndef PET_CONFIG_H
#define PET_CONFIG_H

#include <Arduino.h>

// ===========================================================
// Display
// ===========================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1        // No dedicated reset pin on most SSD1306 modules
#define OLED_I2C_ADDRESS 0x3C

namespace PetConfig {

// ===========================================================
// Face geometry (pixels) - tune Zaito's proportions here.
// ===========================================================
constexpr int16_t FACE_CENTER_X = SCREEN_WIDTH / 2;
constexpr int16_t EYE_CENTER_Y = 26;   // vertical center of the eyes, before breathing/gaze offsets
constexpr int16_t EYE_GAP = 9;         // half-gap between the two eyes (space next to the nose)
constexpr int16_t EYE_WIDTH = 30;      // eye width when fully open
constexpr int16_t EYE_HEIGHT = 34;     // eye height when fully open
constexpr int16_t EYE_CORNER_RADIUS = 10;
constexpr int16_t MOUTH_CENTER_Y = 47;

// How far (pixels) the eyes are allowed to travel when Zaito looks
// around, and how droopy/closed an eye can get.
constexpr int8_t GAZE_RANGE_X = 7;
constexpr int8_t GAZE_RANGE_Y_UP = 4;

// Overlay icon anchor points (top-right corner, clear of the eyes).
// Only one overlay is ever shown at a time, so these can safely share
// roughly the same corner of the screen.
constexpr int16_t OVERLAY_HEART_X = 112, OVERLAY_HEART_Y = 10;
constexpr int16_t OVERLAY_SWEAT_X = 103, OVERLAY_SWEAT_Y = 6;
constexpr int16_t OVERLAY_ANGER_X = 104, OVERLAY_ANGER_Y = 8;
constexpr int16_t OVERLAY_SPARKLE_X = 106, OVERLAY_SPARKLE_Y = 8;
constexpr int16_t OVERLAY_ZZZ_X = 84, OVERLAY_ZZZ_Y = 6;

// ===========================================================
// Animation timing (all in milliseconds). Every "how often" value is a
// [MIN, MAX] range - PetBrain rolls a random value inside it each time,
// so Zaito never looks like it's running on a fixed, robotic loop.
// ===========================================================

// Random blinking
constexpr unsigned long BLINK_INTERVAL_MIN = 2000;
constexpr unsigned long BLINK_INTERVAL_MAX = 6000;
constexpr unsigned long BLINK_DURATION = 180;

// Wink (deliberate, single-eye, held slightly longer than a blink)
constexpr unsigned long WINK_DURATION = 420;

// Idle eye movement (look left/right/up, then back to center)
constexpr unsigned long LOOK_INTERVAL_MIN = 3000;
constexpr unsigned long LOOK_INTERVAL_MAX = 7500;
constexpr unsigned long LOOK_DURATION = 900;

// Random yawning
constexpr unsigned long YAWN_INTERVAL_MIN = 18000;
constexpr unsigned long YAWN_INTERVAL_MAX = 35000;
constexpr unsigned long YAWN_DURATION = 1400;

// Random mood/expression changes
constexpr unsigned long EXPRESSION_INTERVAL_MIN = 4000;
constexpr unsigned long EXPRESSION_INTERVAL_MAX = 9000;

// Falling asleep / waking up on its own
constexpr unsigned long SLEEP_INTERVAL_MIN = 35000;
constexpr unsigned long SLEEP_INTERVAL_MAX = 70000;
constexpr unsigned long SLEEP_DURATION_MIN = 8000;
constexpr unsigned long SLEEP_DURATION_MAX = 16000;
constexpr unsigned long SLEEP_EYE_TRANSITION = 500; // ease eyes shut/open over this long

// Random "nervous" sweat-drop flash (independent of current mood)
constexpr unsigned long SWEAT_INTERVAL_MIN = 12000;
constexpr unsigned long SWEAT_INTERVAL_MAX = 25000;
constexpr unsigned long SWEAT_DURATION = 1500;

// Reaction bursts triggered when entering a mood
constexpr unsigned long JUMP_DURATION = 450;   // happy/excited entry
constexpr unsigned long SHAKE_DURATION = 500;  // laughing entry
constexpr unsigned long OVERLAY_BURST_DURATION = 1500; // heart/anger/sparkle burst length
constexpr int8_t JUMP_HEIGHT = 8;
constexpr int8_t SHAKE_AMPLITUDE = 3;

// Idle breathing sway (continuous, while awake and not jumping/shaking)
constexpr unsigned long BREATH_PERIOD = 3000;
constexpr float BREATH_AMPLITUDE = 1.6f;

// Render throttle - the brain updates every loop(), but the OLED is only
// redrawn at this rate (plenty smooth, and kinder to the I2C bus/CPU).
constexpr unsigned long FRAME_INTERVAL = 33; // ~30 FPS

// ===========================================================
// FUTURE PERIPHERALS
// ---------------------------------------------------------------
// This project is intentionally structured so that adding hardware
// later means adding a few lines HERE plus a small poller in the .ino
// file - not rewriting PetBrain/PetFace. See the README for details.
// Example of what this section will look like once you wire a button:
//
//   constexpr uint8_t BUTTON_PIN = 4;
//   constexpr uint8_t TOUCH_PIN  = T0;
// ===========================================================

} // namespace PetConfig

#endif // PET_CONFIG_H
