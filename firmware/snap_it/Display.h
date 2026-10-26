#pragma once

#include <Adafruit_HX8357.h>
#include <Adafruit_ImageReader.h>
#include <Arduino.h>

#include "GameTypes.h"

class Display {
 public:
  Display();

  bool begin();
  bool prepareRound(size_t imageIndex);
  void showPicker(size_t imageIndex);
  void showCommand(Command command, uint8_t score, uint32_t responseMs);
  void revealNextTiles(uint8_t count);
  void showSuccess(uint8_t score);
  void showGameOver(uint8_t score, bool timedOut);
  void showVictory(uint8_t score);
  void showFault(const char *message);

  bool isReady() const { return ready_; }
  const char *lastError() const { return lastError_; }

 private:
  void drawStatus(const __FlashStringHelper *message, uint8_t score);
  void shuffleTiles();
  void drawTile(uint16_t tileIndex);

  Adafruit_HX8357 tft_;
  Adafruit_ImageReader reader_;
  Adafruit_Image image_;
  uint16_t tileOrder_[990] = {};
  uint16_t revealedTiles_ = 0;
  bool tftStarted_ = false;
  bool ready_ = false;
  const char *lastError_ = "Display has not been initialized";
};

