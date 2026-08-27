/*
 * SoundFX.h
 * ---------
 * Non-blocking buzzer sequencer. Every "sound" is just a short list of
 * (frequency, duration) notes; update() walks through the currently
 * playing sequence purely by comparing millis() against the current
 * note's duration - no delay() anywhere, safe to call every loop().
 *
 * ZeitoBrain never calls this directly (see HomeTypes::SoundEvent) -
 * the top-level sketch/facade is the only thing that turns a
 * SoundEvent into an actual playXxx() call, keeping the behavior layer
 * hardware-agnostic.
 */
#ifndef SOUND_FX_H
#define SOUND_FX_H

#include <Arduino.h>

class SoundFX {
public:
  struct Note {
    uint16_t freq;   // Hz, 0 = silence
    uint16_t durMs;
  };

  void begin();

  // Call every loop() iteration.
  void update(unsigned long now);

  // One function per sound effect - add a new one here + its sequence
  // in SoundFX.cpp to add a new sound later.
  void playWater();
  void playWelcome();
  void playClick();
  void playSelect();
  void playTalk();
  void playPlay();
  void playFeed();

  void stop();

private:
  const Note *_sequence = nullptr;
  uint8_t _length = 0;
  uint8_t _index = 0;
  unsigned long _noteStartAt = 0;
  bool _playing = false;

  void startSequence(const Note *seq, uint8_t len);
  void soundNote(uint8_t index);
};

#endif // SOUND_FX_H
