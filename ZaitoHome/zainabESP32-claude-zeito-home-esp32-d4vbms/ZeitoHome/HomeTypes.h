/*
 * HomeTypes.h
 * -----------
 * Shared enums and the ZeitoViewState struct - the contract between
 * ZeitoBrain (decides what Zeito is doing) and ZeitoView (draws it).
 * ZeitoBrain writes a ZeitoViewState every update(); ZeitoView only
 * ever reads one. Neither side needs to know how the other works.
 */
#ifndef HOME_TYPES_H
#define HOME_TYPES_H

#include <Arduino.h>

// What Zeito is doing right now. One value per behavior; see
// ZeitoBrain.cpp for the transition table (one function per state).
enum class ZeitoActivity : uint8_t {
  WANDER_IDLE,   // standing still somewhere on the floor, about to decide what's next
  WANDER_WALK,   // walking to a random spot just to roam around
  GO_TO_PLANT,   // walking toward the plant
  WATERING,      // standing at the plant, watering it
  GO_TO_BED,     // walking to the bed (bedtime, either automatic or via the menu)
  SLEEPING,      // lying on the bed, eyes closed, occasional dreams
  GO_TO_WINDOW,      // walking to the window because a hand was detected
  GREETING,          // standing at the window, smiling and waving hello
  GO_TO_WINDOW_AUTO, // walking to the window on its own, just to look outside (no hand involved)
  LOOKING_OUT_WINDOW,// standing at the window, calmly looking out (no smile/sound - see GREETING for that)
  WAKING_UP,     // just woke up from a nap - brief smiling beat before resuming the normal routine
  REACT_TALK,    // menu "Talk" - brief speech-bubble reaction
  REACT_PLAY,    // menu "Play" - brief happy reaction
  REACT_FEED,    // menu "Feed" - brief eating reaction
};

// A one-shot sound effect ZeitoBrain wants played. ZeitoBrain never
// touches the buzzer itself - it just raises this flag; the top-level
// sketch (via SoundFX) is responsible for actually making noise. This
// keeps the behavior layer hardware-agnostic, same as the display.
enum class SoundEvent : uint8_t {
  NONE,
  WATER,
  WELCOME,
  TALK,
  PLAY,
  FEED,
};

// Icon shown inside a dream/speech bubble above Zeito's head.
enum class BubbleIcon : uint8_t {
  NONE,
  STAR,
  HEART,
  CAT,
  BEAR,
  FLOWER,
  ROCKET,
  PIZZA,
  NOTE, // used for the "Talk" speech bubble, not a dream
  COUNT // not a real icon - sizes the random dream picker
};

// Everything ZeitoView needs to draw one frame of Zeito. ZeitoBrain
// owns and mutates one instance of this every update(); ZeitoView's
// draw functions only ever read it.
struct ZeitoViewState {
  int16_t x = 0;               // sprite center-x, on the floor baseline
  bool facingRight = true;
  bool isWalking = false;
  bool stepParity = false;     // alternates each pixel step - picks walk1 vs walk2
  bool eyesClosed = false;     // mid-blink, or asleep
  bool smiling = false;
  bool lying = false;          // sleeping pose, drawn on the bed instead of the floor
  uint8_t zzzCount = 0;        // 0 = hidden, 1..3 = how many Z's to show (progressive Z -> ZZ -> ZZZ while falling asleep)
  bool showBubble = false;
  BubbleIcon bubbleIcon = BubbleIcon::NONE;
  bool showWateringCan = false;
};

#endif // HOME_TYPES_H
