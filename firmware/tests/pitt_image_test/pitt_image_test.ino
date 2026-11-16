#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include <Arduino.h>
#include <SPI.h>

#include "PITT01.inc"
#include "PITT02.inc"
#include "PITT03.inc"

namespace {

constexpr uint8_t PIN_TFT_DC = 9;
constexpr uint8_t PIN_TFT_CS = 10;
constexpr uint8_t PIN_SPI_MOSI = 11;
constexpr uint8_t PIN_SPI_SCK = 12;
constexpr uint8_t PIN_TFT_RESET = 14;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t TFT_SPI_HZ = 8000000;
constexpr uint32_t IMAGE_DURATION_MS = 5000;
constexpr int16_t STATUS_HEIGHT = 32;
constexpr int16_t IMAGE_WIDTH = 480;
constexpr int16_t IMAGE_HEIGHT = 288;
constexpr size_t IMAGE_BYTES = IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint16_t);

static_assert(PITT01_RGB565_SIZE == IMAGE_BYTES, "PITT01 has the wrong size");
static_assert(PITT02_RGB565_SIZE == IMAGE_BYTES, "PITT02 has the wrong size");
static_assert(PITT03_RGB565_SIZE == IMAGE_BYTES, "PITT03 has the wrong size");

struct TestImage {
  const char *label;
  const uint8_t *pixels;
};

constexpr TestImage TEST_IMAGES[] = {
    {"PITT CAMPUS", PITT01_RGB565},
    {"HEINZ CHAPEL", PITT02_RGB565},
    {"PITT PANTHER", PITT03_RGB565},
};
constexpr size_t IMAGE_COUNT = sizeof(TEST_IMAGES) / sizeof(TEST_IMAGES[0]);

Adafruit_HX8357 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RESET);

size_t imageIndex = 0;
uint32_t nextImageAtMs = 0;

void drawStatusBar(const TestImage &image) {
  tft.fillRect(0, 0, 480, STATUS_HEIGHT, HX8357_BLUE);
  tft.setTextWrap(false);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(2);
  tft.setCursor(8, 8);
  tft.print(image.label);
  tft.setCursor(418, 8);
  tft.print(imageIndex + 1);
  tft.print('/');
  tft.print(IMAGE_COUNT);
}

void drawCurrentImage(uint32_t nowMs) {
  const TestImage &image = TEST_IMAGES[imageIndex];
  Serial.print(F("Drawing embedded image: "));
  Serial.println(image.label);

  drawStatusBar(image);
  tft.drawRGBBitmap(0, STATUS_HEIGHT,
                    reinterpret_cast<const uint16_t *>(image.pixels),
                    IMAGE_WIDTH, IMAGE_HEIGHT);
  nextImageAtMs = nowMs + IMAGE_DURATION_MS;
}

}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(250);
  Serial.println();
  Serial.println(F("University of Pittsburgh embedded-photo slideshow"));
  Serial.println(F("No microSD card is required."));

  pinMode(PIN_TFT_CS, OUTPUT);
  digitalWrite(PIN_TFT_CS, HIGH);
  SPI.begin(PIN_SPI_SCK, -1, PIN_SPI_MOSI);
  tft.begin(TFT_SPI_HZ);
  tft.setRotation(1);
  tft.fillScreen(HX8357_BLACK);
  drawCurrentImage(millis());
}

void loop() {
  const uint32_t nowMs = millis();
  if (static_cast<int32_t>(nowMs - nextImageAtMs) >= 0) {
    imageIndex = (imageIndex + 1) % IMAGE_COUNT;
    drawCurrentImage(nowMs);
  }
}
