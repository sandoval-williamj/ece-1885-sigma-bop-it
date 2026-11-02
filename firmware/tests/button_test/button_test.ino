#include <Arduino.h>

namespace {

// These pins are free while the Adafruit 2050 display remains connected on
// GPIO 8 through GPIO 14.
constexpr uint8_t PIN_SNAP_BUTTON = 5;
constexpr uint8_t PIN_SNAP_LED = 6;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t DEBOUNCE_MS = 25;
constexpr uint32_t LED_PULSE_MS = 150;

bool lastRawButtonState = HIGH;
bool stableButtonState = HIGH;
uint32_t lastRawChangeAtMs = 0;

bool ledIsOn = false;
uint32_t ledOffAtMs = 0;
uint32_t pressCount = 0;

void startLedPulse(uint32_t nowMs) {
  digitalWrite(PIN_SNAP_LED, HIGH);
  ledIsOn = true;
  ledOffAtMs = nowMs + LED_PULSE_MS;
}

void updateLed(uint32_t nowMs) {
  if (ledIsOn && static_cast<int32_t>(nowMs - ledOffAtMs) >= 0) {
    digitalWrite(PIN_SNAP_LED, LOW);
    ledIsOn = false;
  }
}

void updateButton(uint32_t nowMs) {
  const bool rawButtonState = digitalRead(PIN_SNAP_BUTTON);

  if (rawButtonState != lastRawButtonState) {
    lastRawButtonState = rawButtonState;
    lastRawChangeAtMs = nowMs;
  }

  if (rawButtonState == stableButtonState ||
      nowMs - lastRawChangeAtMs < DEBOUNCE_MS) {
    return;
  }

  stableButtonState = rawButtonState;
  if (stableButtonState == LOW) {
    ++pressCount;
    Serial.print(F("SNAP press #"));
    Serial.println(pressCount);
    startLedPulse(nowMs);
  } else {
    Serial.println(F("SNAP release"));
  }
}

}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(250);

  pinMode(PIN_SNAP_BUTTON, INPUT_PULLUP);
  pinMode(PIN_SNAP_LED, OUTPUT);
  digitalWrite(PIN_SNAP_LED, LOW);

  lastRawButtonState = digitalRead(PIN_SNAP_BUTTON);
  stableButtonState = lastRawButtonState;

  Serial.println();
  Serial.println(F("SNAP button + LED test"));
  Serial.println(F("Button: GPIO 5 to GND (active LOW, internal pull-up)"));
  Serial.println(F("LED: GPIO 6 through 330 ohms to LED anode; cathode to GND"));
  Serial.println(F("Press the button. Each press should produce one 150 ms flash."));
}

void loop() {
  const uint32_t nowMs = millis();
  updateButton(nowMs);
  updateLed(nowMs);
}
