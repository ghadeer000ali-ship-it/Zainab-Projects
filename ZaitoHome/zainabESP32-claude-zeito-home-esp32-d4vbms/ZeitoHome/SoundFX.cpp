#include "SoundFX.h"
#include "HomeConfig.h"

using namespace HomeConfig;

// ===========================================================
// One note sequence per sound effect
// ===========================================================
namespace {
using Note = SoundFX::Note;

const Note WATER_SEQ[] = {
  {1600, 60}, {0, 40}, {1500, 60}, {0, 40}, {1400, 70},
  {0, 50}, {1300, 70}, {0, 50}, {1200, 80}, {0, 300},
};
const Note WELCOME_SEQ[] = {
  {880, 90}, {0, 20}, {988, 90}, {0, 20}, {1175, 90}, {0, 20}, {1568, 150},
};
const Note CLICK_SEQ[] = {
  {1000, 40},
};
const Note SELECT_SEQ[] = {
  {1200, 60}, {0, 30}, {1600, 80},
};
const Note TALK_SEQ[] = {
  {700, 60}, {0, 40}, {900, 60}, {0, 40}, {700, 60},
};
const Note PLAY_SEQ[] = {
  {659, 90}, {784, 90}, {988, 90}, {1319, 140},
};
const Note FEED_SEQ[] = {
  {300, 80}, {0, 40}, {250, 100},
};

template <typename T, uint8_t N>
constexpr uint8_t countOf(const T (&)[N]) { return N; }
} // namespace

void SoundFX::begin() {
  pinMode(PIN_BUZZER, OUTPUT);
  noTone(PIN_BUZZER);
}

void SoundFX::startSequence(const Note *seq, uint8_t len) {
  _sequence = seq;
  _length = len;
  _index = 0;
  _noteStartAt = millis();
  _playing = (len > 0);
  if (_playing) soundNote(0);
  else noTone(PIN_BUZZER);
}

void SoundFX::soundNote(uint8_t index) {
  if (_sequence[index].freq > 0) tone(PIN_BUZZER, _sequence[index].freq);
  else noTone(PIN_BUZZER);
}

void SoundFX::update(unsigned long now) {
  if (!_playing) return;

  if (now - _noteStartAt >= _sequence[_index].durMs) {
    _index++;
    if (_index >= _length) {
      noTone(PIN_BUZZER);
      _playing = false;
      return;
    }
    _noteStartAt = now;
    soundNote(_index);
  }
}

void SoundFX::stop() {
  noTone(PIN_BUZZER);
  _playing = false;
}

void SoundFX::playWater()   { startSequence(WATER_SEQ,   countOf(WATER_SEQ)); }
void SoundFX::playWelcome() { startSequence(WELCOME_SEQ, countOf(WELCOME_SEQ)); }
void SoundFX::playClick()   { startSequence(CLICK_SEQ,   countOf(CLICK_SEQ)); }
void SoundFX::playSelect()  { startSequence(SELECT_SEQ,  countOf(SELECT_SEQ)); }
void SoundFX::playTalk()    { startSequence(TALK_SEQ,    countOf(TALK_SEQ)); }
void SoundFX::playPlay()    { startSequence(PLAY_SEQ,    countOf(PLAY_SEQ)); }
void SoundFX::playFeed()    { startSequence(FEED_SEQ,    countOf(FEED_SEQ)); }
