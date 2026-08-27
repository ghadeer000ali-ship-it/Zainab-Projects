/*
 * PetFace.h
 * ---------
 * The DRAWING layer. PetFace turns a FaceState (produced by PetBrain)
 * into pixels on the SSD1306. It contains no timers, no randomness, and
 * no notion of "what happens next" - give it the same FaceState twice
 * and it draws the exact same frame twice. That purity is what makes it
 * safe to redesign Zaito's look later without touching any behavior
 * code.
 */
#ifndef PET_FACE_H
#define PET_FACE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PetTypes.h"

class PetFace {
public:
  explicit PetFace(Adafruit_SSD1306 &display);

  // Draws one full frame of Zaito onto the display buffer. Does NOT
  // call clearDisplay()/display() - the caller (Pet) controls flushing.
  void render(const FaceState &state);

private:
  Adafruit_SSD1306 *_display;

  // ---- Low-level primitives, shared by every expression ----
  //
  // A filled, optionally slanted, rounded blob - the building block for
  // every eye AND for the wide-open laughing mouth (a mouth is just a
  // short, wide "eye"). Rendered scanline-by-scanline with
  // drawFastHLine() so it stays cheap even on a slow MCU.
  //   side:        -1 = left eye, +1 = right eye, 0 = n/a (mouths).
  //                Controls which corner angrySlant/droopSlant cuts.
  //   angrySlant:  0..1, slices the inner-top corner down and inward.
  //   droopSlant:  0..1, slices the outer-top corner down (half-lidded).
  void drawBlob(int16_t cx, int16_t cy, int16_t w, int16_t h,
                float openness, int16_t cornerRadius, int8_t side = 0,
                float angrySlant = 0.0f, float droopSlant = 0.0f);

  // A parabolic arc stroke - reused for smiling/frowning mouths AND for
  // squinted "happy" eyes. direction=+1 dips at the center (a smile,
  // "U"); direction=-1 peaks at the center (a frown/squint, "∩").
  void drawArc(int16_t cx, int16_t cy, int16_t halfWidth, int16_t depth,
               uint8_t thickness, int8_t direction);

  // A small teardrop hatch mark under an eye (blush / shyness).
  void drawBlush(int16_t cx, int16_t cy, int8_t side);

  // One "Z" glyph, hand-drawn as three strokes (cheaper on flash than
  // pulling in a text font just for this). drawOverlayZzz() calls this
  // three times at shrinking scale.
  void drawZChar(int16_t cx, int16_t cy, float scale);

  // Eye/mouth-center helpers: fold in the current gaze offset and body
  // offset so every drawExpression_xxx() doesn't have to repeat it.
  int16_t leftEyeX(const FaceState &s) const;
  int16_t rightEyeX(const FaceState &s) const;
  int16_t eyeY(const FaceState &s) const;
  int16_t mouthX(const FaceState &s) const;
  int16_t mouthY(const FaceState &s) const;

  // ---- One independent function per expression ----
  // Each one owns both the eyes AND the mouth for its mood, so you can
  // add an 11th expression by writing one new function + one enum value
  // + one line in the render() dispatch switch.
  void drawExpressionNormal(const FaceState &s);
  void drawExpressionHappy(const FaceState &s);
  void drawExpressionSad(const FaceState &s);
  void drawExpressionAngry(const FaceState &s);
  void drawExpressionSleepy(const FaceState &s);
  void drawExpressionExcited(const FaceState &s);
  void drawExpressionLaughing(const FaceState &s);
  void drawExpressionSurprised(const FaceState &s);
  void drawExpressionShy(const FaceState &s);
  void drawExpressionThinking(const FaceState &s);

  // Full-face overrides (see FaceState::isAsleep / isYawning).
  void drawSleepingFace(const FaceState &s);
  void drawYawningFace(const FaceState &s);

  // ---- One independent function per overlay icon ----
  void drawOverlayHeart();
  void drawOverlaySweat();
  void drawOverlayAnger();
  void drawOverlaySparkle();
  void drawOverlayZzz();
};

#endif // PET_FACE_H
