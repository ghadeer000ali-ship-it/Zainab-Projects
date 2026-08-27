/*
 * HomeConfig.h
 * ------------
 * Every pin assignment, display setting, room-geometry constant and
 * timing constant used by the "Zeito Home" project lives here. Nothing
 * in this file is logic - if you want to move the bed, change how long
 * Zeito waits before falling asleep, or rewire a sensor to a different
 * pin, this is the only file you should need to touch.
 */
#ifndef HOME_CONFIG_H
#define HOME_CONFIG_H

#include <Arduino.h>

// ===========================================================
// Display (SSD1306, 128x64, I2C)
// ===========================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C
// SDA -> GPIO21, SCL -> GPIO22 (ESP32 default I2C pins, used by Wire.begin()).

namespace HomeConfig {

// ===========================================================
// Pins
// ===========================================================
constexpr uint8_t PIN_HCSR04_TRIG = 18;
constexpr uint8_t PIN_HCSR04_ECHO = 19;

constexpr uint8_t PIN_JOYSTICK_VRX = 34;
constexpr uint8_t PIN_JOYSTICK_VRY = 35;
constexpr uint8_t PIN_JOYSTICK_SW = 27;

constexpr uint8_t PIN_BUZZER = 26;

// ===========================================================
// Feature toggles
// ===========================================================
// Master switch for the joystick + Talk/Sleep/Play/Feed popup menu.
// Set to `true` to bring them back exactly as they were - nothing else
// needs to change. While `false`:
//   - Joystick.begin()/update() are never called (no pin/ADC reads at
//     all - the joystick hardware can even stay unplugged).
//   - MenuView never opens, updates or draws.
//   - Zeito is fully autonomous (ZeitoBrain's own wander/sleep/plant
//     schedule) and only reacts to the HC-SR04.
// See ZeitoHome.cpp for the three `if (ENABLE_JOYSTICK_MENU)` guards
// this flag controls - Joystick.*/MenuView.* themselves are untouched
// and ready to use the moment this flips back to true.
constexpr bool ENABLE_JOYSTICK_MENU = false;

// Master switch for the HC-SR04 ultrasonic sensor. Set to `true` to
// bring back the hand-wave greeting exactly as it was. While `false`:
//   - UltrasonicSensor.begin()/update() are never called (no pin reads,
//     no pulseIn() wait at all - the sensor can even stay unplugged).
//   - ZeitoBrain::setHandPresence() is simply never called, so nothing
//     ever waits on it or branches on hand proximity - Zeito's routine
//     runs entirely on its own timers (see the wander/plant/window/bed
//     cycle below).
// See ZeitoHome.cpp for the two `if (ENABLE_HC_SR04)` guards this flag
// controls - UltrasonicSensor.* and ZeitoBrain's hand-related code are
// untouched and ready the moment this flips back to true.
constexpr bool ENABLE_HC_SR04 = false;

// ===========================================================
// Render throttle - the brain/sensors/UI update every loop(), but the
// OLED is only redrawn at this rate.
// ===========================================================
constexpr unsigned long FRAME_INTERVAL_MS = 33; // ~30 FPS

// ===========================================================
// Character sprite geometry (pixels) - see ZeitoSprites.h for the
// actual pixel-art bitmaps.
// ===========================================================
constexpr uint8_t SPRITE_W = 8;
constexpr uint8_t SPRITE_H = 12;
constexpr uint8_t ICON_W = 7; // dream/speech bubble icons
constexpr uint8_t ICON_H = 7;

// ===========================================================
// Room layout (pixels) - the whole "scene" Zeito lives in.
// ===========================================================
constexpr int16_t FLOOR_Y = 60;                     // baseboard line
constexpr int16_t CHAR_TOP_Y = FLOOR_Y - SPRITE_H;  // sprite top when standing on the floor

// Window (top-left wall)
constexpr int16_t WINDOW_X = 6, WINDOW_Y = 4, WINDOW_W = 24, WINDOW_H = 20;

// Plant pot (bottom-left, on the floor)
constexpr int16_t PLANT_POT_X = 4, PLANT_POT_Y = 48, PLANT_POT_W = 14, PLANT_POT_H = 12;
constexpr int16_t PLANT_LEAVES_CX = PLANT_POT_X + PLANT_POT_W / 2;
constexpr int16_t PLANT_LEAVES_TOP_Y = 36;

// Bed (bottom-right)
constexpr int16_t BED_X = 82, BED_Y = 34, BED_W = 44, BED_H = 26;
constexpr int16_t BED_PILLOW_X = BED_X + 4, BED_PILLOW_Y = BED_Y + 2;
constexpr int16_t BED_PILLOW_W = 14, BED_PILLOW_H = 8;

// Where Zeito stands (center-x of the sprite) to interact with each spot.
constexpr int16_t STAND_X_WINDOW = 20;
constexpr int16_t STAND_X_PLANT = 24;
constexpr int16_t STAND_X_BED = 100;

// Free-roam wandering bounds (center-x of the sprite).
constexpr int16_t WALK_MIN_X = 24;
constexpr int16_t WALK_MAX_X = 90;

// ===========================================================
// Walking
// ===========================================================
constexpr unsigned long WALK_STEP_INTERVAL_MS = 45; // 1 pixel per tick -> smooth pixel-by-pixel motion

// ===========================================================
// Blinking (independent of activity, except while asleep)
// ===========================================================
constexpr unsigned long BLINK_INTERVAL_MIN_MS = 2500;
constexpr unsigned long BLINK_INTERVAL_MAX_MS = 6000;
constexpr unsigned long BLINK_DURATION_MS = 150;

// ===========================================================
// Idle wandering
// ===========================================================
constexpr unsigned long WANDER_PAUSE_MIN_MS = 1200;
constexpr unsigned long WANDER_PAUSE_MAX_MS = 3500;

// ===========================================================
// Watering the plant
// ===========================================================
constexpr uint8_t PLANT_VISIT_CHANCE_PERCENT = 30; // rolled each time Zeito decides where to wander next
constexpr unsigned long PLANT_VISIT_COOLDOWN_MIN_MS = 15000;
constexpr unsigned long PLANT_VISIT_COOLDOWN_MAX_MS = 30000;
constexpr unsigned long WATERING_DURATION_MIN_MS = 3000; // "tending to the plant" - a fresh random pick each visit
constexpr unsigned long WATERING_DURATION_MAX_MS = 5000;

// ===========================================================
// Looking out the window on its own (autonomous - independent of the
// HC-SR04 hand-triggered greeting, which stays defined but inert while
// ENABLE_HC_SR04 is false - see below).
// ===========================================================
constexpr uint8_t WINDOW_LOOK_CHANCE_PERCENT = 25; // rolled together with PLANT_VISIT_CHANCE_PERCENT - see tickWanderIdle()
constexpr unsigned long WINDOW_LOOK_COOLDOWN_MIN_MS = 12000;
constexpr unsigned long WINDOW_LOOK_COOLDOWN_MAX_MS = 25000;
constexpr unsigned long WINDOW_LOOK_DURATION_MIN_MS = 2000;
constexpr unsigned long WINDOW_LOOK_DURATION_MAX_MS = 3500;

// ===========================================================
// Sleep: Zeito heads to bed after a random stretch of active time, naps
// for a random duration, then always wakes up on its own - no outside
// input is needed for any of this.
// ===========================================================
constexpr unsigned long ACTIVE_PERIOD_MIN_MS = 20000; // how long Zeito stays up before heading to bed
constexpr unsigned long ACTIVE_PERIOD_MAX_MS = 40000;
constexpr unsigned long SLEEP_NAP_DURATION_MIN_MS = 8000; // how long the nap itself lasts
constexpr unsigned long SLEEP_NAP_DURATION_MAX_MS = 12000;
constexpr unsigned long WAKING_UP_DURATION_MIN_MS = 1000; // brief smiling beat right after waking, before resuming the routine
constexpr unsigned long WAKING_UP_DURATION_MAX_MS = 1800;
constexpr unsigned long ZZZ_STAGE_INTERVAL_MS = 700; // Z, then ZZ, then ZZZ - one more Z every this many ms after falling asleep
constexpr unsigned long DREAM_DELAY_MIN_MS = 2000;   // time asleep before the first dream bubble
constexpr unsigned long DREAM_DELAY_MAX_MS = 4000;
constexpr unsigned long DREAM_BUBBLE_DURATION_MS = 3000;
constexpr unsigned long DREAM_CYCLE_MIN_MS = 3000;   // gap before another dream bubble, for longer naps
constexpr unsigned long DREAM_CYCLE_MAX_MS = 5000;

// ===========================================================
// Hand-wave greeting (HC-SR04) - definitions kept intact but never
// exercised while HomeConfig::ENABLE_HC_SR04 is false (the default in
// this version - see the feature toggle above).
// ===========================================================
constexpr float HAND_DETECT_DISTANCE_CM = 20.0f;
constexpr unsigned long GREETING_DURATION_MS = 2500;
constexpr unsigned long GREETING_MAX_DURATION_MS = 15000; // safety cap even if the hand never leaves - see tickGreeting()
constexpr unsigned long HAND_DETECT_COOLDOWN_MS = 5000; // ignore new triggers for this long after one fires

// ===========================================================
// Menu reactions (Talk / Play / Feed)
// ===========================================================
constexpr unsigned long REACTION_DURATION_MS = 1800;

// ===========================================================
// HC-SR04 ultrasonic sensor (only read while ENABLE_HC_SR04 is true -
// see the feature toggle above)
// ===========================================================
constexpr unsigned long ULTRASONIC_PING_INTERVAL_MS = 120;
constexpr unsigned long ULTRASONIC_TIMEOUT_US = 25000UL; // ~4.3m max range

// ===========================================================
// Joystick (only read while ENABLE_JOYSTICK_MENU is true - see above)
// ===========================================================
constexpr int16_t JOYSTICK_CENTER = 2048;
constexpr int16_t JOYSTICK_THRESHOLD = 1200; // distance from center to count as "deflected"
constexpr unsigned long JOYSTICK_REPEAT_MS = 220; // auto-repeat rate while held past threshold
constexpr unsigned long JOYSTICK_DEBOUNCE_MS = 40;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 60;

// ===========================================================
// Menu (Talk / Sleep / Play / Feed popup, only used while
// ENABLE_JOYSTICK_MENU is true - see above)
// ===========================================================
constexpr unsigned long MENU_AUTO_CLOSE_MS = 8000; // closes itself if left untouched

} // namespace HomeConfig

#endif // HOME_CONFIG_H
