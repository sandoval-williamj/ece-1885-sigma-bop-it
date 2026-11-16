#include <Arduino.h>

namespace {

// These pins remain free while the display uses GPIO 8-14 and the SNAP test
// uses GPIO 5-6.
constexpr uint8_t PIN_ENCODER_A = 15;
constexpr uint8_t PIN_ENCODER_B = 16;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr int8_t TRANSITIONS_PER_DETENT = 4;

// Indexed by the previous AB state in bits 3:2 and the current AB state in
// bits 1:0. Valid Gray-code transitions contribute one quarter-step. A bounce
// back to the previous state cancels rather than creating an extra detent.
constexpr int8_t TRANSITION_DELTAS[16] = {
    0,  -1, 1, 0,
    1,  0,  0, -1,
    -1, 0,  0, 1,
    0,  1,  -1, 0,
};

uint8_t previousState = 0;
int8_t transitionAccumulator = 0;
int32_t detentPosition = 0;
uint32_t invalidTransitionCount = 0;

uint8_t readEncoderState() {
  return (digitalRead(PIN_ENCODER_A) << 1) |
         digitalRead(PIN_ENCODER_B);
}

void reportDetent(const __FlashStringHelper *direction) {
  Serial.print(direction);
  Serial.print(F(" detent | position: "));
  Serial.println(detentPosition);
}

void updateEncoder() {
  const uint8_t currentState = readEncoderState();
  if (currentState == previousState) {
    return;
  }

  const uint8_t changedBits = previousState ^ currentState;
  const uint8_t transitionIndex = (previousState << 2) | currentState;
  const int8_t delta = TRANSITION_DELTAS[transitionIndex];
  previousState = currentState;

  // Both channels changing between two reads is not a valid quadrature edge.
  // It usually means a loose wire, contact noise, or rotation too fast for the
  // current wiring quality.
  if (changedBits == 0b11) {
    transitionAccumulator = 0;
    ++invalidTransitionCount;
    Serial.print(F("Invalid AB transition #"));
    Serial.println(invalidTransitionCount);
    return;
  }

  transitionAccumulator += delta;

  if (transitionAccumulator >= TRANSITIONS_PER_DETENT) {
    transitionAccumulator -= TRANSITIONS_PER_DETENT;
    ++detentPosition;
    reportDetent(F("CW"));
  } else if (transitionAccumulator <= -TRANSITIONS_PER_DETENT) {
    transitionAccumulator += TRANSITIONS_PER_DETENT;
    --detentPosition;
    reportDetent(F("CCW"));
  }
}

}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(250);

  pinMode(PIN_ENCODER_A, INPUT_PULLUP);
  pinMode(PIN_ENCODER_B, INPUT_PULLUP);
  previousState = readEncoderState();

  Serial.println();
  Serial.println(F("FOCUS rotary encoder test"));
  Serial.println(F("A: GPIO 15 | C/common: GND | B: GPIO 16"));
  Serial.println(F("Rotate one detent at a time. One revolution is 24 detents."));
  Serial.print(F("Initial AB state: "));
  Serial.print((previousState >> 1) & 1);
  Serial.println(previousState & 1);
}

void loop() { updateEncoder(); }
