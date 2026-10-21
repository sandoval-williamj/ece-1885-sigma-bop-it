#pragma once

#include <Arduino.h>

namespace Config {

// Provisional ESP32 Dev Module pin map. Review before PCB layout.
constexpr uint8_t PIN_SPI_SCK = 18;
constexpr uint8_t PIN_SPI_MISO = 19;
constexpr uint8_t PIN_SPI_MOSI = 23;
constexpr uint8_t PIN_TFT_CS = 4;
constexpr uint8_t PIN_TFT_DC = 16;
constexpr uint8_t PIN_TFT_RESET = 17;
constexpr uint8_t PIN_SD_CS = 13;

constexpr uint8_t PIN_I2C_SDA = 21;
constexpr uint8_t PIN_I2C_SCL = 22;

constexpr uint8_t PIN_I2S_BCLK = 26;
constexpr uint8_t PIN_I2S_LRCLK = 25;
constexpr uint8_t PIN_I2S_DATA = 27;

constexpr uint8_t PIN_SNAP_BUTTON = 32;
constexpr uint8_t PIN_START_BUTTON = 33;
constexpr uint8_t PIN_IMAGE_BUTTON = 14;
constexpr uint8_t PIN_ENCODER_A = 34;
constexpr uint8_t PIN_ENCODER_B = 35;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t BUTTON_DEBOUNCE_MS = 25;
constexpr int64_t ENCODER_COUNTS_PER_ACTION = 2;
constexpr uint16_t IMU_SAMPLE_INTERVAL_MS = 20;
constexpr float SHAKE_DELTA_G = 1.2F;
constexpr uint16_t SHAKE_COOLDOWN_MS = 600;

constexpr uint32_t INITIAL_RESPONSE_MS = 5000;
constexpr uint32_t MINIMUM_RESPONSE_MS = 1000;
constexpr uint32_t RESPONSE_DECREMENT_MS = 40;
constexpr uint16_t SUCCESS_FEEDBACK_MS = 350;
constexpr uint8_t WINNING_SCORE = 99;

constexpr int16_t SCREEN_WIDTH = 480;
constexpr int16_t SCREEN_HEIGHT = 320;
constexpr int16_t STATUS_HEIGHT = 32;
constexpr int16_t IMAGE_WIDTH = 480;
constexpr int16_t IMAGE_HEIGHT = 288;
constexpr uint8_t TILE_COLUMNS = 45;
constexpr uint8_t TILE_ROWS = 22;
constexpr uint16_t TILE_COUNT = TILE_COLUMNS * TILE_ROWS;
constexpr uint8_t TILES_PER_SUCCESS = 10;

constexpr float AUDIO_GAIN = 0.30F;

struct ImageAsset {
  const char *label;
  const char *filename;
};

constexpr ImageAsset IMAGES[] = {
    {"Color Bars", "/IMAGES/IMG01.BMP"},
    {"Sunset", "/IMAGES/IMG02.BMP"},
    {"Mountains", "/IMAGES/IMG03.BMP"},
};

constexpr size_t IMAGE_COUNT = sizeof(IMAGES) / sizeof(IMAGES[0]);

constexpr char AUDIO_SNAP[] = "/AUDIO/SNAP.WAV";
constexpr char AUDIO_FOCUS[] = "/AUDIO/FOCUS.WAV";
constexpr char AUDIO_SHAKE[] = "/AUDIO/SHAKE.WAV";

}  // namespace Config

