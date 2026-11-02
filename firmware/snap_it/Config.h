#pragma once

#include <Arduino.h>

namespace Config {

// Current ESP32-S3 DevKitC-1 bench pin map. A later Feather migration will
// require a new board-specific mapping.

// Adafruit 2050 HX8357D display and its onboard microSD socket. The game now
// uses embedded media, so MISO and SD CS are retained only to keep the unused
// card interface safely deselected and available for possible future use.
// Keep SERIAL_ONLY_MODE true while no working TFT is connected. Set it false
// after replacing the display to restore all screen drawing.
constexpr bool SERIAL_ONLY_MODE = true;
constexpr uint8_t PIN_SPI_SCK = 12;
constexpr uint8_t PIN_SPI_MISO = 13;
constexpr uint8_t PIN_SPI_MOSI = 11;
constexpr uint8_t PIN_TFT_CS = 10;
constexpr uint8_t PIN_TFT_DC = 9;
constexpr uint8_t PIN_TFT_RESET = 14;
constexpr uint8_t PIN_SD_CS = 8;
constexpr uint32_t TFT_SPI_HZ = 8000000;

// Adafruit STEMMA QT MPU-6050 accelerometer and gyroscope I2C bus.
// VIN connects to 3.3 V, AD0 remains open for address 0x68, and INT is unused.
constexpr uint8_t PIN_I2C_SDA = 1;
constexpr uint8_t PIN_I2C_SCL = 2;

// Adafruit MAX98357A mono I2S amplifier used for spoken game commands.
// BCLK, LRC, and DIN are the only required ESP32 signal connections.
constexpr uint8_t PIN_I2S_BCLK = 39;
constexpr uint8_t PIN_I2S_LRCLK = 40;
constexpr uint8_t PIN_I2S_DATA = 41;

// Active-low game pushbuttons. Each button connects its GPIO to common ground;
// firmware enables the ESP32-S3 internal pull-up resistor.
constexpr uint8_t PIN_START_BUTTON = 4;
constexpr uint8_t PIN_SNAP_BUTTON = 5;
constexpr uint8_t PIN_IMAGE_BUTTON = 7;

// SNAP indicator LED. Connect GPIO6 through a 330-ohm resistor to the LED
// anode, with the LED cathode connected to common ground.
constexpr uint8_t PIN_SNAP_LED = 6;

// Bourns Focus rotary encoder. A and B use internal pull-ups; the encoder's
// C/common terminal connects to ground. Rotation direction does not matter.
constexpr uint8_t PIN_ENCODER_A = 15;
constexpr uint8_t PIN_ENCODER_B = 16;

// Optional PT-1520PQ passive buzzer used only by its bench test. The actual
// game voice audio is produced by the MAX98357A amplifier above.
constexpr uint8_t PIN_TEST_BUZZER = 17;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t BUTTON_DEBOUNCE_MS = 25;
constexpr uint16_t SNAP_LED_PULSE_MS = 150;
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

}  // namespace Config

