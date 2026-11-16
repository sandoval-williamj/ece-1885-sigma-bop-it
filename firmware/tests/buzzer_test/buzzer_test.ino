#include <Arduino.h>

namespace {

constexpr uint8_t PIN_BUZZER = 17;
constexpr uint8_t PWM_RESOLUTION_BITS = 10;
constexpr uint32_t TONE_DURATION_MS = 350;
constexpr uint32_t GAP_DURATION_MS = 250;
constexpr uint32_t CYCLE_PAUSE_MS = 3000;

constexpr uint16_t TONE_FREQUENCIES_HZ[] = {1500, 2000, 2500};
constexpr size_t TONE_COUNT =
    sizeof(TONE_FREQUENCIES_HZ) / sizeof(TONE_FREQUENCIES_HZ[0]);

enum class TestState : uint8_t { Tone, Gap, CyclePause };

TestState state = TestState::Tone;
size_t toneIndex = 0;
uint32_t nextChangeAtMs = 0;
bool pwmReady = false;

void startTone(uint32_t nowMs) {
  const uint16_t frequency = TONE_FREQUENCIES_HZ[toneIndex];
  ledcWriteTone(PIN_BUZZER, frequency);
  Serial.print(F("Tone: "));
  Serial.print(frequency);
  Serial.println(F(" Hz"));
  state = TestState::Tone;
  nextChangeAtMs = nowMs + TONE_DURATION_MS;
}

void stopTone() { ledcWriteTone(PIN_BUZZER, 0); }

void updateTest(uint32_t nowMs) {
  if (static_cast<int32_t>(nowMs - nextChangeAtMs) < 0) {
    return;
  }

  switch (state) {
    case TestState::Tone:
      stopTone();
      state = TestState::Gap;
      nextChangeAtMs = nowMs + GAP_DURATION_MS;
      break;

    case TestState::Gap:
      ++toneIndex;
      if (toneIndex < TONE_COUNT) {
        startTone(nowMs);
      } else {
        toneIndex = 0;
        state = TestState::CyclePause;
        Serial.println(F("Cycle complete; pausing for 3 seconds."));
        nextChangeAtMs = nowMs + CYCLE_PAUSE_MS;
      }
      break;

    case TestState::CyclePause:
      startTone(nowMs);
      break;
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(250);

  Serial.println();
  Serial.println(F("PT-1520PQ passive piezo test"));
  Serial.println(F("GPIO 17 through 100-220 ohms to buzzer +; buzzer - to GND"));

  if (!ledcAttach(PIN_BUZZER, TONE_FREQUENCIES_HZ[0],
                  PWM_RESOLUTION_BITS)) {
    Serial.println(F("ERROR: could not attach the LEDC PWM peripheral"));
    return;
  }

  pwmReady = true;
  startTone(millis());
}

void loop() {
  if (pwmReady) {
    updateTest(millis());
  }
}
