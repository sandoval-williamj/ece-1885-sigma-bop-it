#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include <Adafruit_ImageReader.h>
#include <Arduino.h>
#include <SPI.h>
#include <SdFat_Adafruit_Fork.h>

// ESP32-S3-DevKitC-1 display bring-up map. GPIO 11/12/13 are the S3's
// native SPI2 MOSI/SCK/MISO pins. GPIO 19/20 remain free for native USB.
constexpr uint8_t PIN_SPI_SCK = 12;
constexpr uint8_t PIN_SPI_MISO = 13;
constexpr uint8_t PIN_SPI_MOSI = 11;
constexpr uint8_t PIN_TFT_CS = 10;
constexpr uint8_t PIN_TFT_DC = 9;
constexpr uint8_t PIN_TFT_RESET = 14;
constexpr uint8_t PIN_SD_CS = 8;

Adafruit_HX8357 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RESET);
SdFat mediaCard;
Adafruit_ImageReader imageReader(mediaCard);

void showMessage(const char *title, const char *detail, uint16_t color) {
  tft.fillScreen(HX8357_BLACK);
  tft.setTextWrap(true);
  tft.setTextColor(color);
  tft.setTextSize(3);
  tft.setCursor(16, 24);
  tft.println(title);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(2);
  tft.setCursor(16, 80);
  tft.println(detail);
}

void setup() {
  Serial.begin(115200);
  delay(250);
  Serial.println();
  Serial.println(F("HX8357 display bring-up test"));

  // Deselect both devices before starting their shared SPI bus.
  pinMode(PIN_TFT_CS, OUTPUT);
  digitalWrite(PIN_TFT_CS, HIGH);
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);

  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
  tft.begin(HX8357D);
  tft.setRotation(1);

  // These flashes prove that the TFT works even if the card does not.
  tft.fillScreen(HX8357_RED);
  delay(300);
  tft.fillScreen(HX8357_GREEN);
  delay(300);
  tft.fillScreen(HX8357_BLUE);
  delay(300);
  showMessage("DISPLAY OK", "Checking the microSD card...", HX8357_GREEN);

  if (!mediaCard.begin(PIN_SD_CS, SD_SCK_MHZ(20))) {
    Serial.println(F("ERROR: microSD initialization failed"));
    showMessage("DISPLAY OK",
                "SD FAILED\nCheck FAT32 format, card seating, MISO, and CCS/SDCS wiring.",
                HX8357_RED);
    return;
  }

  Serial.println(F("microSD initialized"));
  tft.fillScreen(HX8357_BLACK);
  tft.setTextWrap(false);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(2);
  tft.setCursor(6, 8);
  tft.print(F("DISPLAY + SD OK: IMG01.BMP"));

  const ImageReturnCode result =
      imageReader.drawBMP("/IMAGES/IMG01.BMP", tft, 0, 32);
  if (result != IMAGE_SUCCESS) {
    Serial.println(F("ERROR: could not draw /IMAGES/IMG01.BMP"));
    imageReader.printStatus(result);
    showMessage("DISPLAY + SD OK",
                "IMAGE FAILED\nCopy assets/sdcard/IMAGES to the card root.",
                HX8357_RED);
    return;
  }

  Serial.println(F("Image displayed successfully"));
}

void loop() {}
