/*
 * UltrasonicSensor.h
 * ------------------
 * Thin, throttled driver for the HC-SR04. update() only actually pings
 * the sensor every HomeConfig::ULTRASONIC_PING_INTERVAL_MS - it is safe
 * (and intended) to call update() every loop() iteration.
 *
 * Note on pulseIn(): reading the echo pin is inherently a "wait for the
 * pin to change" operation. We bound it with a timeout
 * (HomeConfig::ULTRASONIC_TIMEOUT_US, a few milliseconds) and only run
 * it once per ping interval rather than every loop() - the rest of the
 * sketch (brain, joystick, buzzer, rendering) is entirely millis()-based
 * and unaffected between pings. A fully interrupt-driven echo read
 * would remove even that bounded wait, but isn't needed for a hand-wave
 * proximity check like this one.
 */
#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>

class UltrasonicSensor {
public:
  void begin();

  // Call every loop() iteration; internally throttled.
  void update(unsigned long now);

  // Last successfully measured distance in cm, or -1 if the last ping
  // timed out (nothing in range).
  float lastDistanceCm() const { return _lastDistanceCm; }

  // True if the last measured distance is closer than thresholdCm.
  bool isNear(float thresholdCm) const;

private:
  unsigned long _lastPingAt = 0;
  float _lastDistanceCm = -1.0f;

  float ping();
};

#endif // ULTRASONIC_SENSOR_H
