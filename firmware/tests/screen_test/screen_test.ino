#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include <Arduino.h>
#include <SPI.h>

// ESP32-S3-DevKitC-1 to Adafruit 2050 HX8357D.
constexpr uint8_t PIN_SPI_SCK = 12;
constexpr uint8_t PIN_SPI_MOSI = 11;
constexpr uint8_t PIN_TFT_CS = 10;
constexpr uint8_t PIN_TFT_DC = 9;
constexpr uint8_t PIN_TFT_RESET = 14;

Adafruit_HX8357 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RESET);

void showColor(uint16_t color, const char *name, uint16_t textColor) {
  tft.fillScreen(color);
  tft.setTextWrap(false);
  tft.setTextColor(textColor);
  tft.setTextSize(4);
  tft.setCursor(40, 130);
  tft.print(name);
  delay(2000);
}

void setup() {
  Serial.begin(115200);
  delay(250);
  Serial.println();
  Serial.println(F("Starting HX8357 screen-only test"));

  SPI.begin(PIN_SPI_SCK, -1, PIN_SPI_MOSI);
  tft.begin(HX8357D);
  tft.setRotation(1);
}

void loop() {
  showColor(HX8357_RED, "RED", HX8357_WHITE);
  showColor(HX8357_GREEN, "GREEN", HX8357_BLACK);
  showColor(HX8357_BLUE, "BLUE", HX8357_WHITE);
  showColor(HX8357_WHITE, "WHITE", HX8357_BLACK);
  showColor(HX8357_BLACK, "SCREEN WORKS", HX8357_CYAN);
}
