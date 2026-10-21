#include "Display.h"

#include <SPI.h>
#include <esp32-hal-psram.h>
#include <esp_system.h>
#include <string.h>

#include "Config.h"
#include "MediaCard.h"

static_assert(Config::TILE_COUNT == 990,
              "The reveal grid must contain exactly 990 tiles.");

Display::Display()
    : tft_(Config::PIN_TFT_CS, Config::PIN_TFT_DC, Config::PIN_TFT_RESET),
      reader_(mediaCard) {}

bool Display::begin() {
  SPI.begin(Config::PIN_SPI_SCK, Config::PIN_SPI_MISO, Config::PIN_SPI_MOSI);
  tft_.begin();
  tft_.setRotation(1);
  tft_.setTextWrap(false);
  tftStarted_ = true;
  tft_.fillScreen(HX8357_BLACK);

  if (!psramFound()) {
    lastError_ = "PSRAM is required but was not detected";
    showFault(lastError_);
    return false;
  }

  if (!beginMediaCard()) {
    lastError_ = "microSD initialization failed";
    showFault(lastError_);
    return false;
  }

  ready_ = true;
  lastError_ = "";
  return true;
}

void Display::showPicker(size_t imageIndex) {
  tft_.fillScreen(HX8357_BLACK);
  tft_.setTextColor(HX8357_WHITE);
  tft_.setTextSize(3);
  tft_.setCursor(24, 60);
  tft_.print(F("SELECT IMAGE"));

  tft_.setTextColor(HX8357_CYAN);
  tft_.setTextSize(4);
  tft_.setCursor(24, 130);
  tft_.print(Config::IMAGES[imageIndex].label);

  tft_.setTextColor(HX8357_WHITE);
  tft_.setTextSize(2);
  tft_.setCursor(24, 205);
  tft_.print(imageIndex + 1);
  tft_.print(F(" / "));
  tft_.print(Config::IMAGE_COUNT);
  tft_.setCursor(24, 260);
  tft_.print(F("IMAGE cycles   START plays"));
}

bool Display::prepareRound(size_t imageIndex) {
  if (!ready_ || imageIndex >= Config::IMAGE_COUNT) {
    lastError_ = "Invalid display or image selection state";
    return false;
  }

  const ImageReturnCode result =
      reader_.loadBMP(Config::IMAGES[imageIndex].filename, image_);
  if (result != IMAGE_SUCCESS) {
    lastError_ = "Image could not be loaded from microSD";
    Serial.print(F("ERROR loading "));
    Serial.println(Config::IMAGES[imageIndex].filename);
    reader_.printStatus(result);
    showFault(lastError_);
    return false;
  }

  if (image_.getFormat() != IMAGE_16 ||
      image_.width() != Config::IMAGE_WIDTH ||
      image_.height() != Config::IMAGE_HEIGHT) {
    lastError_ = "Image must be an uncompressed 480x288 BMP";
    showFault(lastError_);
    return false;
  }

  tft_.fillScreen(HX8357_BLACK);
  shuffleTiles();
  revealedTiles_ = 0;
  return true;
}

void Display::showCommand(Command command, uint8_t score,
                          uint32_t responseMs) {
  drawStatus(commandLabel(command), score);
  Serial.print(F("Command: "));
  Serial.print(commandLabel(command));
  Serial.print(F(" | response window after speech: "));
  Serial.print(responseMs);
  Serial.println(F(" ms"));
}

void Display::drawStatus(const __FlashStringHelper *message, uint8_t score) {
  tft_.fillRect(0, 0, Config::SCREEN_WIDTH, Config::STATUS_HEIGHT,
                HX8357_BLACK);
  tft_.setTextSize(2);
  tft_.setTextColor(HX8357_WHITE);
  tft_.setCursor(6, 8);
  tft_.print(message);
  tft_.setCursor(385, 8);
  tft_.print(F("S:"));
  tft_.print(score);
}

void Display::shuffleTiles() {
  for (uint16_t index = 0; index < Config::TILE_COUNT; ++index) {
    tileOrder_[index] = index;
  }
  for (uint16_t remaining = Config::TILE_COUNT - 1; remaining > 0;
       --remaining) {
    const uint16_t swapWith = esp_random() % (remaining + 1);
    const uint16_t temporary = tileOrder_[remaining];
    tileOrder_[remaining] = tileOrder_[swapWith];
    tileOrder_[swapWith] = temporary;
  }
}

void Display::revealNextTiles(uint8_t count) {
  while (count-- > 0 && revealedTiles_ < Config::TILE_COUNT) {
    drawTile(tileOrder_[revealedTiles_++]);
  }
}

void Display::drawTile(uint16_t tileIndex) {
  auto *canvas = static_cast<GFXcanvas16 *>(image_.getCanvas());
  if (canvas == nullptr) {
    return;
  }

  const uint16_t column = tileIndex % Config::TILE_COLUMNS;
  const uint16_t row = tileIndex / Config::TILE_COLUMNS;
  const int16_t x0 = column * Config::IMAGE_WIDTH / Config::TILE_COLUMNS;
  const int16_t x1 =
      (column + 1) * Config::IMAGE_WIDTH / Config::TILE_COLUMNS;
  const int16_t y0 = row * Config::IMAGE_HEIGHT / Config::TILE_ROWS;
  const int16_t y1 =
      (row + 1) * Config::IMAGE_HEIGHT / Config::TILE_ROWS;
  const int16_t width = x1 - x0;
  const int16_t height = y1 - y0;

  constexpr int16_t kMaxTileWidth =
      (Config::IMAGE_WIDTH + Config::TILE_COLUMNS - 1) /
      Config::TILE_COLUMNS;
  constexpr int16_t kMaxTileHeight =
      (Config::IMAGE_HEIGHT + Config::TILE_ROWS - 1) / Config::TILE_ROWS;
  uint16_t tilePixels[kMaxTileWidth * kMaxTileHeight];
  const uint16_t *imagePixels = canvas->getBuffer();

  for (int16_t sourceRow = 0; sourceRow < height; ++sourceRow) {
    memcpy(&tilePixels[sourceRow * width],
           &imagePixels[(y0 + sourceRow) * Config::IMAGE_WIDTH + x0],
           width * sizeof(uint16_t));
  }

  tft_.drawRGBBitmap(x0, Config::STATUS_HEIGHT + y0, tilePixels, width,
                     height);
}

void Display::showSuccess(uint8_t score) {
  drawStatus(F("NICE!"), score);
}

void Display::showGameOver(uint8_t score, bool timedOut) {
  drawStatus(timedOut ? F("TIME'S UP") : F("WRONG MOVE"), score);
  Serial.print(F("Round over. Score: "));
  Serial.println(score);
}

void Display::showVictory(uint8_t score) {
  drawStatus(F("PHOTO COMPLETE!"), score);
  Serial.println(F("Victory: the complete image is visible."));
}

void Display::showFault(const char *message) {
  Serial.print(F("HARDWARE FAULT: "));
  Serial.println(message);
  if (!tftStarted_) {
    return;
  }
  tft_.fillScreen(HX8357_RED);
  tft_.setTextColor(HX8357_WHITE);
  tft_.setTextSize(2);
  tft_.setTextWrap(true);
  tft_.setCursor(16, 24);
  tft_.println(F("HARDWARE FAULT"));
  tft_.setCursor(16, 72);
  tft_.println(message);
  tft_.setCursor(16, 160);
  tft_.println(F("See Serial and docs/testing.md"));
  tft_.setTextWrap(false);
}

