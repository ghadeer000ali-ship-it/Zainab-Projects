#include "UltrasonicSensor.h"
#include "HomeConfig.h"

using namespace HomeConfig;

void UltrasonicSensor::begin() {
  pinMode(PIN_HCSR04_TRIG, OUTPUT);
  pinMode(PIN_HCSR04_ECHO, INPUT);
  digitalWrite(PIN_HCSR04_TRIG, LOW);
}

void UltrasonicSensor::update(unsigned long now) {
  if (now - _lastPingAt < ULTRASONIC_PING_INTERVAL_MS) return;
  _lastPingAt = now;
  _lastDistanceCm = ping();
}

bool UltrasonicSensor::isNear(float thresholdCm) const {
  return (_lastDistanceCm > 0.0f) && (_lastDistanceCm < thresholdCm);
}

float UltrasonicSensor::ping() {
  digitalWrite(PIN_HCSR04_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_HCSR04_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_HCSR04_TRIG, LOW);

  unsigned long durationUs = pulseIn(PIN_HCSR04_ECHO, HIGH, ULTRASONIC_TIMEOUT_US);
  if (durationUs == 0) return -1.0f; // timed out - nothing in range

  return (float)durationUs * 0.0343f / 2.0f; // speed of sound ~343 m/s
}
