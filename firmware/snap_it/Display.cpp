#include "Display.h"

#include <SPI.h>
#include <esp_system.h>

#include "Config.h"
#include "EmbeddedAssets.h"

static_assert(Config::TILE_COUNT == 990,
              "The reveal grid must contain exactly 990 tiles.");

Display::Display()
    : tft_(Config::PIN_TFT_CS, Config::PIN_TFT_DC, Config::PIN_TFT_RESET) {}

bool Display::begin() {
  if (Config::SERIAL_ONLY_MODE) {
    ready_ = true;
    lastError_ = "";
    Serial.println(F("Display bypassed: SERIAL-ONLY GAME MODE"));
    Serial.println(F("Disconnect the damaged TFT before playing."));
    return true;
  }

  // The card socket is not used, but holding its chip select high prevents it
  // from driving the SPI bus while the TFT is active.
  pinMode(Config::PIN_SD_CS, OUTPUT);
  digitalWrite(Config::PIN_SD_CS, HIGH);
  pinMode(Config::PIN_TFT_CS, OUTPUT);
  digitalWrite(Config::PIN_TFT_CS, HIGH);

  SPI.begin(Config::PIN_SPI_SCK, Config::PIN_SPI_MISO,
            Config::PIN_SPI_MOSI);
  tft_.begin(Config::TFT_SPI_HZ);
  tft_.setRotation(1);
  tft_.setTextWrap(false);
  tftStarted_ = true;
  tft_.fillScreen(HX8357_BLACK);

  ready_ = true;
  lastError_ = "";
  Serial.println(F("Display ready; images are embedded in flash."));
  return true;
}

void Display::showPicker(size_t imageIndex) {
  if (imageIndex >= EmbeddedAssets::IMAGE_COUNT) {
    showFault("Invalid embedded image index");
    return;
  }

  Serial.println();
  Serial.println(F("=== IMAGE SELECT ==="));
  Serial.print(F("Selected: "));
  Serial.print(EmbeddedAssets::IMAGES[imageIndex].label);
  Serial.print(F(" ("));
  Serial.print(imageIndex + 1);
  Serial.print('/');
  Serial.print(EmbeddedAssets::IMAGE_COUNT);
  Serial.println(')');
  Serial.println(F("Press IMAGE to cycle or START to play."));

  if (Config::SERIAL_ONLY_MODE) {
    return;
  }

  tft_.fillScreen(HX8357_BLACK);
  tft_.setTextColor(HX8357_WHITE);
  tft_.setTextSize(3);
  tft_.setCursor(24, 60);
  tft_.print(F("SELECT IMAGE"));

  tft_.setTextColor(HX8357_CYAN);
  tft_.setTextSize(4);
  tft_.setCursor(24, 130);
  tft_.print(EmbeddedAssets::IMAGES[imageIndex].label);

  tft_.setTextColor(HX8357_WHITE);
  tft_.setTextSize(2);
  tft_.setCursor(24, 205);
  tft_.print(imageIndex + 1);
  tft_.print(F(" / "));
  tft_.print(EmbeddedAssets::IMAGE_COUNT);
  tft_.setCursor(24, 260);
  tft_.print(F("IMAGE cycles   START plays"));
}

bool Display::prepareRound(size_t imageIndex) {
  if (!ready_ || imageIndex >= EmbeddedAssets::IMAGE_COUNT) {
    lastError_ = "Invalid display or image selection state";
    return false;
  }

  imagePixels_ = EmbeddedAssets::IMAGES[imageIndex].pixels;
  if (imagePixels_ == nullptr) {
    lastError_ = "Embedded image data is unavailable";
    return false;
  }

  if (!Config::SERIAL_ONLY_MODE) {
    tft_.fillScreen(HX8357_BLACK);
  }
  shuffleTiles();
  revealedTiles_ = 0;
  Serial.print(F("Round image: "));
  Serial.println(EmbeddedAssets::IMAGES[imageIndex].label);
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
  if (Config::SERIAL_ONLY_MODE) {
    return;
  }

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
    if (!Config::SERIAL_ONLY_MODE) {
      drawTile(tileOrder_[revealedTiles_]);
    }
    ++revealedTiles_;
  }
  Serial.print(F("Reveal progress: "));
  Serial.print(revealedTiles_);
  Serial.print('/');
  Serial.println(Config::TILE_COUNT);
}

void Display::drawTile(uint16_t tileIndex) {
  if (imagePixels_ == nullptr) {
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

  constexpr int16_t MAX_TILE_WIDTH =
      (Config::IMAGE_WIDTH + Config::TILE_COLUMNS - 1) /
      Config::TILE_COLUMNS;
  constexpr int16_t MAX_TILE_HEIGHT =
      (Config::IMAGE_HEIGHT + Config::TILE_ROWS - 1) / Config::TILE_ROWS;
  uint16_t tilePixels[MAX_TILE_WIDTH * MAX_TILE_HEIGHT];
  const uint16_t *sourcePixels =
      reinterpret_cast<const uint16_t *>(imagePixels_);

  for (int16_t sourceRow = 0; sourceRow < height; ++sourceRow) {
    for (int16_t sourceColumn = 0; sourceColumn < width; ++sourceColumn) {
      const size_t sourceIndex =
          static_cast<size_t>(y0 + sourceRow) * Config::IMAGE_WIDTH + x0 +
          sourceColumn;
      tilePixels[sourceRow * width + sourceColumn] =
          pgm_read_word(sourcePixels + sourceIndex);
    }
  }

  tft_.drawRGBBitmap(x0, Config::STATUS_HEIGHT + y0, tilePixels, width,
                     height);
}

void Display::showSuccess(uint8_t score) {
  drawStatus(F("NICE!"), score);
  Serial.print(F("CORRECT | score: "));
  Serial.println(score);
}

void Display::showGameOver(uint8_t score, bool timedOut) {
  drawStatus(timedOut ? F("TIME'S UP") : F("WRONG MOVE"), score);
  Serial.print(F("Round over. Score: "));
  Serial.println(score);
  Serial.println(F("Press START to replay or IMAGE to change photo."));
}

void Display::showVictory(uint8_t score) {
  drawStatus(F("PHOTO COMPLETE!"), score);
  Serial.println(F("Victory: the complete image is visible."));
  Serial.println(F("Press START to replay or IMAGE to change photo."));
}

void Display::showFault(const char *message) {
  Serial.print(F("HARDWARE FAULT: "));
  Serial.println(message);
  if (Config::SERIAL_ONLY_MODE || !tftStarted_) {
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
