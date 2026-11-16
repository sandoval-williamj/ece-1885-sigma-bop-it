#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include <Arduino.h>
#include <ESP32Encoder.h>
#include <SPI.h>

namespace {

constexpr uint8_t PIN_SNAP_BUTTON = 5;
constexpr uint8_t PIN_SNAP_LED = 6;

constexpr uint8_t PIN_SD_CS = 8;
constexpr uint8_t PIN_TFT_DC = 9;
constexpr uint8_t PIN_TFT_CS = 10;
constexpr uint8_t PIN_SPI_MOSI = 11;
constexpr uint8_t PIN_SPI_SCK = 12;
constexpr uint8_t PIN_SPI_MISO = 13;
constexpr uint8_t PIN_TFT_RESET = 14;

constexpr uint8_t PIN_ENCODER_A = 15;
constexpr uint8_t PIN_ENCODER_B = 16;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 25;
constexpr uint32_t LED_PULSE_MS = 150;
constexpr int64_t ENCODER_COUNTS_PER_DETENT = 2;
constexpr uint32_t TFT_SPI_HZ = 8000000;

Adafruit_HX8357 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RESET);
ESP32Encoder encoder;

bool lastRawButtonState = HIGH;
bool stableButtonState = HIGH;
uint32_t lastRawButtonChangeAtMs = 0;

bool ledIsOn = false;
uint32_t ledOffAtMs = 0;

uint32_t snapCount = 0;
int32_t focusPosition = 0;
int64_t reportedEncoderCount = 0;

void printValueAt(int16_t x, int16_t y, int16_t width, int64_t value) {
  tft.fillRect(x, y, width, 28, HX8357_BLACK);
  tft.setCursor(x, y);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(3);
  tft.print(value);
}

void showEvent(const __FlashStringHelper *message, uint16_t color) {
  tft.fillRect(16, 92, 448, 50, HX8357_BLACK);
  tft.drawRect(16, 92, 448, 50, color);
  tft.setCursor(30, 104);
  tft.setTextColor(color);
  tft.setTextSize(3);
  tft.print(message);
}

void drawDashboard() {
  tft.fillScreen(HX8357_BLACK);
  tft.setTextWrap(false);

  tft.fillRect(0, 0, 480, 52, HX8357_BLUE);
  tft.setCursor(14, 14);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(3);
  tft.print(F("INPUT INTEGRATION"));

  tft.setCursor(16, 66);
  tft.setTextColor(HX8357_CYAN);
  tft.setTextSize(2);
  tft.print(F("LAST EVENT"));
  showEvent(F("READY"), HX8357_GREEN);

  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(2);
  tft.setCursor(16, 164);
  tft.print(F("SNAP COUNT:"));
  tft.setCursor(16, 206);
  tft.print(F("FOCUS POSITION:"));
  tft.setCursor(16, 248);
  tft.print(F("ENCODER RAW:"));

  printValueAt(245, 160, 210, snapCount);
  printValueAt(285, 202, 170, focusPosition);
  printValueAt(245, 244, 210, encoder.getCount());

  tft.fillRect(0, 286, 480, 34, HX8357_BLUE);
  tft.setCursor(12, 296);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(1);
  tft.print(F("GPIO 5 SNAP   |   GPIO 15/16 FOCUS   |   microSD not required"));
}

void runDisplayStartupTest() {
  Serial.println(F("TFT startup: RED"));
  tft.fillScreen(HX8357_RED);
  delay(300);
  Serial.println(F("TFT startup: GREEN"));
  tft.fillScreen(HX8357_GREEN);
  delay(300);
  Serial.println(F("TFT startup: BLUE"));
  tft.fillScreen(HX8357_BLUE);
  delay(300);
  Serial.println(F("TFT startup: drawing dashboard"));
  drawDashboard();
  Serial.println(F("TFT startup: dashboard drawn"));
}

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

void handleSnapPress(uint32_t nowMs) {
  ++snapCount;
  startLedPulse(nowMs);
  showEvent(F("SNAP"), HX8357_YELLOW);
  printValueAt(245, 160, 210, snapCount);

  Serial.print(F("SNAP press #"));
  Serial.println(snapCount);
}

void updateButton(uint32_t nowMs) {
  const bool rawButtonState = digitalRead(PIN_SNAP_BUTTON);
  if (rawButtonState != lastRawButtonState) {
    lastRawButtonState = rawButtonState;
    lastRawButtonChangeAtMs = nowMs;
  }

  if (rawButtonState == stableButtonState ||
      nowMs - lastRawButtonChangeAtMs < BUTTON_DEBOUNCE_MS) {
    return;
  }

  stableButtonState = rawButtonState;
  if (stableButtonState == LOW) {
    handleSnapPress(nowMs);
  }
}

void updateEncoder() {
  const int64_t encoderCount = encoder.getCount();
  const int64_t countDelta = encoderCount - reportedEncoderCount;
  if (llabs(countDelta) < ENCODER_COUNTS_PER_DETENT) {
    return;
  }

  const int32_t detentDelta =
      static_cast<int32_t>(countDelta / ENCODER_COUNTS_PER_DETENT);
  focusPosition += detentDelta;
  reportedEncoderCount +=
      static_cast<int64_t>(detentDelta) * ENCODER_COUNTS_PER_DETENT;

  const bool clockwise = detentDelta > 0;
  showEvent(clockwise ? F("FOCUS CW") : F("FOCUS CCW"), HX8357_CYAN);
  printValueAt(285, 202, 170, focusPosition);
  printValueAt(245, 244, 210, encoderCount);

  Serial.print(clockwise ? F("FOCUS CW") : F("FOCUS CCW"));
  Serial.print(F(" | detents: "));
  Serial.print(detentDelta);
  Serial.print(F(" | position: "));
  Serial.println(focusPosition);
}

}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(250);

  Serial.println();
  Serial.println(F("Display + SNAP + FOCUS integration test"));
  Serial.println(F("The microSD card is not used by this sketch."));

  pinMode(PIN_SNAP_BUTTON, INPUT_PULLUP);
  pinMode(PIN_SNAP_LED, OUTPUT);
  digitalWrite(PIN_SNAP_LED, LOW);
  lastRawButtonState = digitalRead(PIN_SNAP_BUTTON);
  stableButtonState = lastRawButtonState;

  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  encoder.attachHalfQuad(PIN_ENCODER_A, PIN_ENCODER_B);
  encoder.setCount(0);

  // Deselect both devices before starting their shared SPI bus. Keep the
  // untested microSD device inactive for the entire test.
  pinMode(PIN_TFT_CS, OUTPUT);
  digitalWrite(PIN_TFT_CS, HIGH);
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
  tft.begin(TFT_SPI_HZ);
  tft.setRotation(1);
  runDisplayStartupTest();
}

void loop() {
  const uint32_t nowMs = millis();
  updateButton(nowMs);
  updateLed(nowMs);
  updateEncoder();
}
