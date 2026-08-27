/*
 * PetFace.cpp
 * -----------
 * Pure rendering: FaceState in, pixels out. See PetFace.h for the
 * design rationale. Every shape is built from just two cheap
 * primitives - a filled/slanted rounded blob (drawBlob) and a
 * parabolic arc stroke (drawArc) - drawn with drawFastHLine()/
 * drawPixel(), so nothing here depends on any Adafruit_GFX helper
 * whose exact pixel behaviour we'd have to guess at.
 */

#include "PetFace.h"
#include "PetConfig.h"
#include <math.h>

PetFace::PetFace(Adafruit_SSD1306 &display) : _display(&display) {}

// ===========================================================
// Low-level primitives
// ===========================================================

void PetFace::drawBlob(int16_t cx, int16_t cy, int16_t w, int16_t h,
                        float openness, int16_t cornerRadius, int8_t side,
                        float angrySlant, float droopSlant) {
  if (openness < 0.0f) openness = 0.0f;
  if (openness > 1.0f) openness = 1.0f;

  float hEff = h * openness;
  if (hEff < 2.0f) hEff = 2.0f;

  // Fully (or near-fully) closed: always render as a simple thin lid
  // line, regardless of the base shape. Trying to squash an arc or a
  // slanted blob down to near-zero height produces ugly artifacts, and
  // a closed eye looks the same no matter which mood it closed from.
  if (hEff <= h * 0.16f) {
    float hw = w * 0.42f;
    int16_t x0 = (int16_t)lroundf(cx - hw);
    int16_t len = (int16_t)lroundf(hw * 2.0f);
    for (int8_t t = -1; t <= 1; t++) {
      _display->drawFastHLine(x0, cy + t, len, SSD1306_WHITE);
    }
    return;
  }

  float halfH = hEff / 2.0f;
  float r = min(min((float)cornerRadius, w / 2.0f), halfH);

  int16_t rowStart = (int16_t)(-halfH);
  int16_t rowEnd = (int16_t)(halfH);

  for (int16_t row = rowStart; row <= rowEnd; row++) {
    float dy = fabsf((float)row);
    float hw;
    if (dy > halfH - r) {
      float pen = dy - (halfH - r);
      float underSqrt = r * r - pen * pen;
      if (underSqrt < 0.0f) underSqrt = 0.0f;
      hw = (w / 2.0f - r) + sqrtf(underSqrt);
    } else {
      hw = w / 2.0f;
    }

    float leftX = cx - hw;
    float rightX = cx + hw;

    // Angry: slice the corner facing the nose down and inward as the
    // row approaches the top -> an inward, glaring eyebrow line.
    if (angrySlant > 0.0f && row < -halfH * 0.15f) {
      float t = (-row - halfH * 0.15f) / (halfH * 0.85f);
      float cut = t * w * angrySlant;
      if (side < 0) rightX -= cut;      // left eye: inner edge is the right edge
      else if (side > 0) leftX += cut;  // right eye: inner edge is the left edge
    }

    // Sleepy/shy: slice the OUTER-top corner down -> half-lidded look.
    if (droopSlant > 0.0f && row < -halfH * 0.1f) {
      float t = (-row - halfH * 0.1f) / (halfH * 0.9f);
      float cut = t * w * droopSlant;
      if (side < 0) leftX += cut;
      else if (side > 0) rightX -= cut;
    }

    if (rightX >= leftX) {
      int16_t xi = (int16_t)lroundf(leftX);
      int16_t len = (int16_t)lroundf(rightX - leftX) + 1;
      if (len > 0) _display->drawFastHLine(xi, cy + row, len, SSD1306_WHITE);
    }
  }
}

void PetFace::drawArc(int16_t cx, int16_t cy, int16_t halfWidth,
                       int16_t depth, uint8_t thickness, int8_t direction) {
  if (halfWidth < 1) halfWidth = 1;
  for (int16_t x = -halfWidth; x <= halfWidth; x++) {
    float norm = (float)x / (float)halfWidth;
    float yOff = direction * depth * (1.0f - norm * norm);
    float yc = cy + yOff;
    for (uint8_t t = 0; t < thickness; t++) {
      int16_t py = (int16_t)lroundf(yc + t - thickness / 2.0f);
      _display->drawPixel(cx + x, py, SSD1306_WHITE);
    }
  }
}

void PetFace::drawBlush(int16_t cx, int16_t cy, int8_t side) {
  for (int8_t i = 0; i < 3; i++) {
    int8_t off = i * 3;
    for (int8_t t = 0; t < 3; t++) {
      _display->drawPixel(cx + side * (off + t), cy + t, SSD1306_WHITE);
    }
  }
}

void PetFace::drawZChar(int16_t cx, int16_t cy, float scale) {
  int16_t s = (int16_t)lroundf(scale);
  if (s < 1) s = 1;
  _display->drawFastHLine(cx - s, cy - s, 2 * s + 1, SSD1306_WHITE);
  int16_t steps = max((int16_t)2, (int16_t)(2 * s));
  for (int16_t i = 0; i <= steps; i++) {
    float xx = cx + s - (2.0f * s) * i / steps;
    float yy = cy - s + (2.0f * s) * i / steps;
    _display->drawPixel((int16_t)lroundf(xx), (int16_t)lroundf(yy), SSD1306_WHITE);
  }
  _display->drawFastHLine(cx - s, cy + s, 2 * s + 1, SSD1306_WHITE);
}

// ===========================================================
// Position helpers
// ===========================================================
int16_t PetFace::leftEyeX(const FaceState &s) const {
  return PetConfig::FACE_CENTER_X - PetConfig::EYE_GAP -
         PetConfig::EYE_WIDTH / 2 + s.bodyOffsetX + s.gazeX;
}
int16_t PetFace::rightEyeX(const FaceState &s) const {
  return PetConfig::FACE_CENTER_X + PetConfig::EYE_GAP +
         PetConfig::EYE_WIDTH / 2 + s.bodyOffsetX + s.gazeX;
}
int16_t PetFace::eyeY(const FaceState &s) const {
  return PetConfig::EYE_CENTER_Y + s.bodyOffsetY + s.gazeY;
}
int16_t PetFace::mouthX(const FaceState &s) const {
  return PetConfig::FACE_CENTER_X + s.bodyOffsetX;
}
int16_t PetFace::mouthY(const FaceState &s) const {
  return PetConfig::MOUTH_CENTER_Y + s.bodyOffsetY;
}

// ===========================================================
// render() - top-level dispatch
// ===========================================================
void PetFace::render(const FaceState &s) {
  if (s.isAsleep) {
    drawSleepingFace(s);
  } else if (s.isYawning) {
    drawYawningFace(s);
  } else {
    switch (s.expression) {
      case Expression::NORMAL:    drawExpressionNormal(s);    break;
      case Expression::HAPPY:     drawExpressionHappy(s);     break;
      case Expression::SAD:       drawExpressionSad(s);       break;
      case Expression::ANGRY:     drawExpressionAngry(s);     break;
      case Expression::SLEEPY:    drawExpressionSleepy(s);    break;
      case Expression::EXCITED:   drawExpressionExcited(s);   break;
      case Expression::LAUGHING:  drawExpressionLaughing(s);  break;
      case Expression::SURPRISED: drawExpressionSurprised(s); break;
      case Expression::SHY:       drawExpressionShy(s);       break;
      case Expression::THINKING:  drawExpressionThinking(s);  break;
      default:                    drawExpressionNormal(s);    break;
    }
  }

  switch (s.overlay) {
    case OverlayType::HEART:   drawOverlayHeart();   break;
    case OverlayType::SWEAT:   drawOverlaySweat();   break;
    case OverlayType::ANGER:   drawOverlayAnger();   break;
    case OverlayType::SPARKLE: drawOverlaySparkle(); break;
    case OverlayType::ZZZ:     drawOverlayZzz();     break;
    default: break;
  }
}

// ===========================================================
// One function per expression
// ===========================================================
using namespace PetConfig;

void PetFace::drawExpressionNormal(const FaceState &s) {
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s);
  drawBlob(lx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessL, EYE_CORNER_RADIUS, -1);
  drawBlob(rx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessR, EYE_CORNER_RADIUS, 1);
  _display->drawFastHLine(mouthX(s) - 8, mouthY(s), 17, SSD1306_WHITE);
}

void PetFace::drawExpressionHappy(const FaceState &s) {
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s);
  drawBlob(lx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessL, EYE_CORNER_RADIUS, -1);
  drawBlob(rx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessR, EYE_CORNER_RADIUS, 1);
  drawArc(mouthX(s), mouthY(s) - 2, 12, 5, 3, +1);
}

void PetFace::drawExpressionSad(const FaceState &s) {
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s) + 2;
  drawBlob(lx, cy, EYE_WIDTH, (int16_t)(EYE_HEIGHT * 0.85f), s.eyeOpennessL,
           EYE_CORNER_RADIUS, -1, 0.0f, 0.35f);
  drawBlob(rx, cy, EYE_WIDTH, (int16_t)(EYE_HEIGHT * 0.85f), s.eyeOpennessR,
           EYE_CORNER_RADIUS, 1, 0.0f, 0.35f);
  drawArc(mouthX(s), mouthY(s) + 4, 10, 4, 3, -1);
}

void PetFace::drawExpressionAngry(const FaceState &s) {
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s);
  drawBlob(lx, cy, EYE_WIDTH, (int16_t)(EYE_HEIGHT * 0.8f), s.eyeOpennessL,
           6, -1, 0.55f, 0.0f);
  drawBlob(rx, cy, EYE_WIDTH, (int16_t)(EYE_HEIGHT * 0.8f), s.eyeOpennessR,
           6, 1, 0.55f, 0.0f);
  _display->drawFastHLine(mouthX(s) - 9, mouthY(s), 19, SSD1306_WHITE);
  _display->drawFastHLine(mouthX(s) - 9, mouthY(s) + 1, 19, SSD1306_WHITE);
}

void PetFace::drawExpressionSleepy(const FaceState &s) {
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s);
  drawBlob(lx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessL * 0.45f,
           EYE_CORNER_RADIUS, -1, 0.0f, 0.25f);
  drawBlob(rx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessR * 0.45f,
           EYE_CORNER_RADIUS, 1, 0.0f, 0.25f);
  drawArc(mouthX(s), mouthY(s) + 1, 5, 2, 3, +1);
}

void PetFace::drawExpressionExcited(const FaceState &s) {
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s);
  drawBlob(lx, cy, (int16_t)(EYE_WIDTH * 1.1f), (int16_t)(EYE_HEIGHT * 1.1f),
           s.eyeOpennessL, 12, -1);
  drawBlob(rx, cy, (int16_t)(EYE_WIDTH * 1.1f), (int16_t)(EYE_HEIGHT * 1.1f),
           s.eyeOpennessR, 12, 1);
  drawArc(mouthX(s), mouthY(s) - 2, 15, 9, 4, +1);
  _display->drawFastHLine(mouthX(s) - 12, mouthY(s) + 7, 25, SSD1306_WHITE);
}

void PetFace::drawExpressionLaughing(const FaceState &s) {
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s);
  // Squinted "^‿^" eyes: reuse the arc primitive instead of the blob.
  drawArc(lx, cy, (int16_t)(EYE_WIDTH * 0.42f), 7, 4, -1);
  drawArc(rx, cy, (int16_t)(EYE_WIDTH * 0.42f), 7, 4, -1);
  // Wide open laughing mouth: a short, wide "eye" blob.
  drawBlob(mouthX(s), mouthY(s) + 4, 26, 14, 1.0f, 6);
}

void PetFace::drawExpressionSurprised(const FaceState &s) {
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s);
  drawBlob(lx, cy, (int16_t)(EYE_WIDTH * 1.05f), (int16_t)(EYE_HEIGHT * 1.15f),
           s.eyeOpennessL, 15, -1);
  drawBlob(rx, cy, (int16_t)(EYE_WIDTH * 1.05f), (int16_t)(EYE_HEIGHT * 1.15f),
           s.eyeOpennessR, 15, 1);
  // Round "O" mouth, drawn as a ring of points.
  int16_t mx = mouthX(s), my = mouthY(s) + 3;
  for (int16_t ang = 0; ang < 360; ang += 10) {
    float rad = ang * (float)PI / 180.0f;
    _display->drawPixel(mx + (int16_t)lroundf(5 * cosf(rad)),
                         my + (int16_t)lroundf(5 * sinf(rad)), SSD1306_WHITE);
  }
}

void PetFace::drawExpressionShy(const FaceState &s) {
  // Eyes look down-and-inward (bashful gaze) - kept visually distinct
  // from "sleepy" by NOT using droopSlant here.
  int16_t lx = leftEyeX(s) + 2, rx = rightEyeX(s) - 2, cy = eyeY(s) + 4;
  drawBlob(lx, cy, (int16_t)(EYE_WIDTH * 0.85f), (int16_t)(EYE_HEIGHT * 0.8f),
           s.eyeOpennessL, EYE_CORNER_RADIUS, -1);
  drawBlob(rx, cy, (int16_t)(EYE_WIDTH * 0.85f), (int16_t)(EYE_HEIGHT * 0.8f),
           s.eyeOpennessR, EYE_CORNER_RADIUS, 1);
  drawArc(mouthX(s), mouthY(s) - 1, 6, 2, 3, +1);
  drawBlush(lx - EYE_WIDTH / 2 - 6, cy + 10, -1);
  drawBlush(rx + EYE_WIDTH / 2 + 2, cy + 10, 1);
}

void PetFace::drawExpressionThinking(const FaceState &s) {
  // Gaze (folded into leftEyeX/rightEyeX via s.gazeX/gazeY) drifts up
  // and to one side, as if looking away in thought.
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s);
  drawBlob(lx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessL, EYE_CORNER_RADIUS, -1);
  drawBlob(rx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessR, EYE_CORNER_RADIUS, 1);
  _display->drawFastHLine(mouthX(s) - 2, mouthY(s) + 2, 8, SSD1306_WHITE);
}

// ===========================================================
// Full-face overrides
// ===========================================================
void PetFace::drawSleepingFace(const FaceState &s) {
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s);
  // eyeOpennessL/R are driven down near 0 by PetBrain during sleep, so
  // this naturally renders as the closed-lid-line shape.
  drawBlob(lx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessL, EYE_CORNER_RADIUS, -1);
  drawBlob(rx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessR, EYE_CORNER_RADIUS, 1);
  drawArc(mouthX(s), mouthY(s) + 1, 5, 2, 3, +1);
}

void PetFace::drawYawningFace(const FaceState &s) {
  int16_t lx = leftEyeX(s), rx = rightEyeX(s), cy = eyeY(s);
  drawBlob(lx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessL, EYE_CORNER_RADIUS, -1);
  drawBlob(rx, cy, EYE_WIDTH, EYE_HEIGHT, s.eyeOpennessR, EYE_CORNER_RADIUS, 1);
  // Tall, narrow open-mouth oval - a "yawn" is a mouth blob rotated 90°
  // relative to the wide "laugh" mouth blob.
  drawBlob(mouthX(s), mouthY(s) + 3, 14, 20, 1.0f, 7);
}

// ===========================================================
// One function per overlay icon
// ===========================================================
void PetFace::drawOverlayHeart() {
  int16_t cx = OVERLAY_HEART_X, cy = OVERLAY_HEART_Y;
  for (int16_t x = -6; x <= 6; x++) {
    for (int16_t y = -6; y <= 6; y++) {
      float xf = x / 5.0f, yf = y / 5.0f;
      float v = powf(xf * xf + yf * yf - 1.0f, 3) - xf * xf * yf * yf * yf;
      if (v <= 0.0f) _display->drawPixel(cx + x, cy + y, SSD1306_WHITE);
    }
  }
}

void PetFace::drawOverlaySweat() {
  int16_t cx = OVERLAY_SWEAT_X, cy = OVERLAY_SWEAT_Y;
  for (int16_t row = -4; row <= 5; row++) {
    float hw = (row < -1) ? (row + 4) * 0.9f : 3.0f - fabsf(row - 2) * 0.5f;
    int16_t x0 = (int16_t)lroundf(cx - hw);
    int16_t len = (int16_t)lroundf(hw * 2.0f) + 1;
    if (len > 0) _display->drawFastHLine(x0, cy + row, len, SSD1306_WHITE);
  }
}

void PetFace::drawOverlayAnger() {
  int16_t cx = OVERLAY_ANGER_X, cy = OVERLAY_ANGER_Y;
  for (int8_t d = -1; d <= 1; d += 2) {
    for (int8_t i = -5; i <= 5; i++) {
      for (int8_t w = 0; w <= 1; w++) {
        _display->drawPixel(cx + i, cy + (int16_t)lroundf(d * i * 0.7f) + w, SSD1306_WHITE);
      }
    }
  }
}

void PetFace::drawOverlaySparkle() {
  int16_t cx = OVERLAY_SPARKLE_X, cy = OVERLAY_SPARKLE_Y;
  static const int16_t angles[8] = {0, 90, 180, 270, 45, 135, 225, 315};
  for (uint8_t i = 0; i < 8; i++) {
    float rad = angles[i] * (float)PI / 180.0f;
    float rr = (i < 4) ? 5.0f : 2.5f;
    for (int16_t step = 1; step <= (int16_t)rr; step++) {
      _display->drawPixel(cx + (int16_t)lroundf(step * cosf(rad)),
                           cy + (int16_t)lroundf(step * sinf(rad)), SSD1306_WHITE);
    }
  }
}

void PetFace::drawOverlayZzz() {
  // Gentle upward drift, purely cosmetic - the DECISION to show Zzz
  // (and for how long) belongs to PetBrain; only this tiny idle bob is
  // computed here from millis() directly, since it carries no
  // behavioral meaning of its own.
  float bob = sinf(millis() / 400.0f) * 1.0f;
  int16_t x = OVERLAY_ZZZ_X, y = OVERLAY_ZZZ_Y + (int16_t)lroundf(bob);
  drawZChar(x, y, 3.0f);
  drawZChar(x + 8, y - 8, 2.2f);
  drawZChar(x + 14, y - 14, 1.6f);
}
