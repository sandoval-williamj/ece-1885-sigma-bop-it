#include <Arduino.h>
#include <ESP_I2S.h>
#include <math.h>
#include <string.h>

namespace {

// These pins are exposed and currently unused on the ESP32-S3 DevKitC-1.
// They do not conflict with the display, SNAP button/LED, encoder, or buzzer.
constexpr uint8_t PIN_I2S_BCLK = 39;
constexpr uint8_t PIN_I2S_LRC = 40;
constexpr uint8_t PIN_I2S_DATA = 41;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t SAMPLE_RATE_HZ = 22050;
constexpr float TEST_LEVEL = 0.08F;  // 8% of digital full scale for first test.
constexpr uint32_t FADE_MS = 20;
constexpr uint32_t BETWEEN_TONES_MS = 150;
constexpr uint32_t BETWEEN_CYCLES_MS = 3000;
constexpr size_t FRAMES_PER_BUFFER = 256;

struct TestTone {
  const char *label;
  uint16_t frequencyHz;
  uint16_t durationMs;
};

constexpr TestTone TEST_TONES[] = {
    {"low", 440, 550},
    {"middle", 660, 550},
    {"high", 880, 700},
};
constexpr size_t TONE_COUNT = sizeof(TEST_TONES) / sizeof(TEST_TONES[0]);

I2SClass i2s;
int16_t stereoBuffer[FRAMES_PER_BUFFER * 2];
bool i2sReady = false;
uint32_t cycleNumber = 0;

bool writeStereoFrames(size_t frameCount) {
  const size_t byteCount = frameCount * 2 * sizeof(int16_t);
  const size_t bytesWritten = i2s.write(stereoBuffer, byteCount);
  if (bytesWritten == byteCount) {
    return true;
  }

  Serial.print(F("ERROR: I2S wrote "));
  Serial.print(bytesWritten);
  Serial.print(F(" of "));
  Serial.print(byteCount);
  Serial.println(F(" bytes"));
  return false;
}

bool playSilence(uint32_t durationMs) {
  memset(stereoBuffer, 0, sizeof(stereoBuffer));
  uint32_t framesRemaining =
      (static_cast<uint64_t>(SAMPLE_RATE_HZ) * durationMs) / 1000;

  while (framesRemaining > 0) {
    const size_t frameCount =
        framesRemaining < FRAMES_PER_BUFFER ? framesRemaining
                                            : FRAMES_PER_BUFFER;
    if (!writeStereoFrames(frameCount)) {
      return false;
    }
    framesRemaining -= frameCount;
  }
  return true;
}

bool playSineTone(uint16_t frequencyHz, uint32_t durationMs) {
  const uint32_t totalFrames =
      (static_cast<uint64_t>(SAMPLE_RATE_HZ) * durationMs) / 1000;
  const uint32_t fadeFrames =
      (static_cast<uint64_t>(SAMPLE_RATE_HZ) * FADE_MS) / 1000;
  const float phaseStep =
      (2.0F * PI * static_cast<float>(frequencyHz)) / SAMPLE_RATE_HZ;
  const float peakSample = TEST_LEVEL * 32767.0F;

  float phase = 0.0F;
  uint32_t firstFrame = 0;
  while (firstFrame < totalFrames) {
    const size_t frameCount =
        (totalFrames - firstFrame) < FRAMES_PER_BUFFER
            ? totalFrames - firstFrame
            : FRAMES_PER_BUFFER;

    for (size_t i = 0; i < frameCount; ++i) {
      const uint32_t frame = firstFrame + i;
      float envelope = 1.0F;
      if (fadeFrames > 0 && frame < fadeFrames) {
        envelope = static_cast<float>(frame + 1) / fadeFrames;
      }
      const uint32_t framesFromEnd = totalFrames - frame;
      if (fadeFrames > 0 && framesFromEnd <= fadeFrames) {
        const float fadeOut = static_cast<float>(framesFromEnd) / fadeFrames;
        if (fadeOut < envelope) {
          envelope = fadeOut;
        }
      }

      const int16_t sample =
          static_cast<int16_t>(sinf(phase) * peakSample * envelope);
      // Identical samples on both channels work with the MAX98357A in left,
      // right, or default stereo-average mode.
      stereoBuffer[i * 2] = sample;
      stereoBuffer[i * 2 + 1] = sample;

      phase += phaseStep;
      if (phase >= 2.0F * PI) {
        phase -= 2.0F * PI;
      }
    }

    if (!writeStereoFrames(frameCount)) {
      return false;
    }
    firstFrame += frameCount;
  }
  return true;
}

void runTestCycle() {
  ++cycleNumber;
  Serial.println();
  Serial.print(F("Speaker test cycle #"));
  Serial.println(cycleNumber);

  for (size_t i = 0; i < TONE_COUNT; ++i) {
    const TestTone &tone = TEST_TONES[i];
    Serial.print(F("Playing "));
    Serial.print(tone.label);
    Serial.print(F(" tone: "));
    Serial.print(tone.frequencyHz);
    Serial.println(F(" Hz"));

    if (!playSineTone(tone.frequencyHz, tone.durationMs) ||
        !playSilence(BETWEEN_TONES_MS)) {
      i2sReady = false;
      Serial.println(F("Test stopped after an I2S write failure."));
      return;
    }
  }

  Serial.println(F("Cycle complete; three seconds of silence."));
  if (!playSilence(BETWEEN_CYCLES_MS)) {
    i2sReady = false;
    Serial.println(F("Test stopped after an I2S write failure."));
  }
}

}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(250);

  Serial.println();
  Serial.println(F("MAX98357A I2S amplifier + speaker test"));
  Serial.println(F("BCLK GPIO39 | LRC GPIO40 | DIN GPIO41"));
  Serial.println(F("Starting at 8% digital full scale."));

  i2s.setPins(PIN_I2S_BCLK, PIN_I2S_LRC, PIN_I2S_DATA);
  i2sReady = i2s.begin(I2S_MODE_STD, SAMPLE_RATE_HZ,
                       I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
  if (!i2sReady) {
    Serial.print(F("ERROR: I2S initialization failed, code "));
    Serial.println(i2s.lastError());
    return;
  }

  Serial.println(F("I2S ready. The three-tone test will repeat."));
  playSilence(100);
}

void loop() {
  if (!i2sReady) {
    delay(1000);
    return;
  }
  runTestCycle();
}
