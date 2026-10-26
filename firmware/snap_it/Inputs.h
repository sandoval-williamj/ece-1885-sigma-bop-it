#pragma once

#include <Adafruit_MPU6050.h>
#include <Arduino.h>
#include <ESP32Encoder.h>

#include "GameTypes.h"

class Inputs {
 public:
  bool begin();
  void update(uint32_t nowMs);

  bool takeStartPress();
  bool takeImagePress();
  bool takeGameplayAction(Command &action);
  void resetGameplayEvents(uint32_t nowMs);
  bool isReady() const { return ready_; }

 private:
  class DebouncedButton {
   public:
    void begin(uint8_t pin);
    void update(uint32_t nowMs);
    bool takePress();

   private:
    uint8_t pin_ = 0;
    bool rawPressed_ = false;
    bool stablePressed_ = false;
    bool pressPending_ = false;
    uint32_t lastChangeMs_ = 0;
  };

  DebouncedButton snapButton_;
  DebouncedButton startButton_;
  DebouncedButton imageButton_;
  ESP32Encoder encoder_;
  Adafruit_MPU6050 mpu_;

  bool ready_ = false;
  bool gameplayActionPending_ = false;
  Command pendingGameplayAction_ = Command::Snap;
  int64_t lastEncoderCount_ = 0;
  uint32_t lastImuSampleMs_ = 0;
  uint32_t nextShakeAllowedMs_ = 0;
};

