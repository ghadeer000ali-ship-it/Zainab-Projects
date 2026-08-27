/*
 * PetTypes.h
 * ----------
 * Shared enums and the FaceState struct - the "contract" between
 * PetBrain (decides what Zaito is doing) and PetFace (draws it).
 *
 * PetBrain writes a FaceState every update(); PetFace only ever reads
 * one. Neither side needs to know how the other is implemented, which
 * is what keeps animation logic and drawing logic independent (you can
 * rewrite the entire rendering style in PetFace.cpp without touching a
 * single timer in PetBrain.cpp, and vice versa).
 */
#ifndef PET_TYPES_H
#define PET_TYPES_H

#include <Arduino.h>

// Zaito's current mood/expression. Each value maps to exactly one
// drawExpression_xxx() function in PetFace.
enum class Expression : uint8_t {
  NORMAL = 0,
  HAPPY,
  SAD,
  ANGRY,
  SLEEPY,
  EXCITED,
  LAUGHING,
  SURPRISED,
  SHY,
  THINKING,
  COUNT // not a real expression - used to size arrays / bound random picks
};

// A small floating icon shown near the top of the screen. At most one
// is ever active at a time, which keeps the screen readable and keeps
// PetBrain's bookkeeping simple.
enum class OverlayType : uint8_t {
  NONE = 0,
  HEART,    // happiness / excitement
  SWEAT,    // nervousness
  ANGER,    // annoyance
  SPARKLE,  // surprise
  ZZZ,      // sleeping
};

// Top-level wake/sleep state. Deliberately separate from Expression:
// "sleepy" is a *mood* Zaito can have while awake (droopy eyes, more
// likely to yawn), whereas SLEEPING is a distinct mode with its own
// closed-eyes-and-Zzz rendering and its own auto-wake timer.
enum class PetState : uint8_t {
  AWAKE,
  SLEEPING,
};

// Everything PetFace needs to draw one frame. PetBrain owns and mutates
// one instance of this every update(); PetFace::render() only reads it.
struct FaceState {
  Expression expression = Expression::NORMAL;

  // 0.0 = fully closed, 1.0 = fully open. Independent per eye so a wink
  // only affects one side.
  float eyeOpennessL = 1.0f;
  float eyeOpennessR = 1.0f;

  // Shared eye "gaze" offset in pixels (both eyes move together, like a
  // real pair of eyes).
  int8_t gazeX = 0;
  int8_t gazeY = 0;

  // Whole-face offset in pixels, used for breathing, jumping and
  // shaking without any of the drawing code needing to know why the
  // face moved.
  int8_t bodyOffsetX = 0;
  int8_t bodyOffsetY = 0;

  // Full-face overrides that temporarily replace the normal
  // per-expression mouth/eyes (see PetFace::render()).
  bool isAsleep = false;
  bool isYawning = false;

  OverlayType overlay = OverlayType::NONE;
};

#endif // PET_TYPES_H
