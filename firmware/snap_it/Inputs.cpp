#include "Inputs.h"

#include <Wire.h>
#include <math.h>

#include "Config.h"

void Inputs::DebouncedButton::begin(uint8_t pin) {
  pin_ = pin;
  pinMode(pin_, INPUT_PULLUP);
  rawPressed_ = digitalRead(pin_) == LOW;
  stablePressed_ = rawPressed_;
  lastChangeMs_ = millis();
}

void Inputs::DebouncedButton::update(uint32_t nowMs) {
  const bool pressed = digitalRead(pin_) == LOW;
  if (pressed != rawPressed_) {
    rawPressed_ = pressed;
    lastChangeMs_ = nowMs;
  }

  if (stablePressed_ != rawPressed_ &&
      nowMs - lastChangeMs_ >= Config::BUTTON_DEBOUNCE_MS) {
    stablePressed_ = rawPressed_;
    if (stablePressed_) {
      pressPending_ = true;
    }
  }
}

bool Inputs::DebouncedButton::takePress() {
  const bool pressed = pressPending_;
  pressPending_ = false;
  return pressed;
}

bool Inputs::begin() {
  snapButton_.begin(Config::PIN_SNAP_BUTTON);
  startButton_.begin(Config::PIN_START_BUTTON);
  imageButton_.begin(Config::PIN_IMAGE_BUTTON);

  pinMode(Config::PIN_ENCODER_A, INPUT);
  pinMode(Config::PIN_ENCODER_B, INPUT);
  ESP32Encoder::useInternalWeakPullResistors = puType::none;
  encoder_.attachHalfQuad(Config::PIN_ENCODER_A, Config::PIN_ENCODER_B);
  encoder_.setCount(0);

  Wire.begin(Config::PIN_I2C_SDA, Config::PIN_I2C_SCL);
  if (!mpu_.begin(0x68, &Wire)) {
    Serial.println(F("ERROR: MPU-6050 was not found."));
    return false;
  }

  mpu_.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu_.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu_.setFilterBandwidth(MPU6050_BAND_21_HZ);
  ready_ = true;
  return true;
}

void Inputs::update(uint32_t nowMs) {
  snapButton_.update(nowMs);
  startButton_.update(nowMs);
  imageButton_.update(nowMs);

  if (snapButton_.takePress() && !gameplayActionPending_) {
    pendingGameplayAction_ = Command::Snap;
    gameplayActionPending_ = true;
  }

  const int64_t encoderCount = encoder_.getCount();
  if (llabs(encoderCount - lastEncoderCount_) >=
      Config::ENCODER_COUNTS_PER_ACTION) {
    if (!gameplayActionPending_) {
      pendingGameplayAction_ = Command::Focus;
      gameplayActionPending_ = true;
    }
    lastEncoderCount_ = encoderCount;
  }

  if (ready_ && nowMs - lastImuSampleMs_ >= Config::IMU_SAMPLE_INTERVAL_MS) {
    lastImuSampleMs_ = nowMs;
    sensors_event_t acceleration;
    sensors_event_t rotation;
    sensors_event_t temperature;
    mpu_.getEvent(&acceleration, &rotation, &temperature);

    constexpr float kGravity = 9.80665F;
    const float magnitude = sqrtf(
        acceleration.acceleration.x * acceleration.acceleration.x +
        acceleration.acceleration.y * acceleration.acceleration.y +
        acceleration.acceleration.z * acceleration.acceleration.z);
    const float deltaG = fabsf(magnitude / kGravity - 1.0F);
    if (deltaG >= Config::SHAKE_DELTA_G &&
        static_cast<int32_t>(nowMs - nextShakeAllowedMs_) >= 0) {
      if (!gameplayActionPending_) {
        pendingGameplayAction_ = Command::Shake;
        gameplayActionPending_ = true;
      }
      nextShakeAllowedMs_ = nowMs + Config::SHAKE_COOLDOWN_MS;
    }
  }
}

bool Inputs::takeStartPress() { return startButton_.takePress(); }

bool Inputs::takeImagePress() { return imageButton_.takePress(); }

bool Inputs::takeGameplayAction(Command &action) {
  if (!gameplayActionPending_) {
    return false;
  }

  action = pendingGameplayAction_;
  gameplayActionPending_ = false;
  return true;
}

void Inputs::resetGameplayEvents(uint32_t nowMs) {
  gameplayActionPending_ = false;
  lastEncoderCount_ = encoder_.getCount();
  nextShakeAllowedMs_ = nowMs + 250;
}

