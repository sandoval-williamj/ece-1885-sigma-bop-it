#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

namespace {

// Reserved free pins on the current ESP32-S3 DevKitC-1 bench build.
constexpr uint8_t PIN_IMU_SDA = 1;
constexpr uint8_t PIN_IMU_SCL = 2;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t I2C_FREQUENCY_HZ = 100000;
constexpr uint32_t SENSOR_SAMPLE_INTERVAL_MS = 20;
constexpr uint32_t SERIAL_PRINT_INTERVAL_MS = 100;
constexpr float STANDARD_GRAVITY_MPS2 = 9.80665F;
constexpr float RADIANS_TO_DEGREES = 180.0F / PI;

// These are the same Shake parameters currently used by snap_it.
constexpr float SHAKE_DELTA_G = 1.2F;
constexpr uint32_t SHAKE_COOLDOWN_MS = 600;

constexpr uint8_t ADDRESS_DEFAULT = 0x68;
constexpr uint8_t ADDRESS_AD0_HIGH = 0x69;

Adafruit_MPU6050 mpu;
bool imuReady = false;
uint8_t imuAddress = 0;
uint32_t nextSampleAtMs = 0;
uint32_t nextPrintAtMs = 0;
uint32_t nextShakeAllowedAtMs = 0;
uint32_t shakeCount = 0;
uint32_t nextFaultMessageAtMs = 0;

void scanI2cBus() {
  Serial.println(F("Scanning I2C bus..."));
  uint8_t deviceCount = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  Device at 0x"));
      if (address < 0x10) {
        Serial.print('0');
      }
      Serial.println(address, HEX);
      ++deviceCount;
    }
  }
  if (deviceCount == 0) {
    Serial.println(F("  No I2C devices found."));
  }
}

bool beginMpu6050() {
  if (mpu.begin(ADDRESS_DEFAULT, &Wire)) {
    imuAddress = ADDRESS_DEFAULT;
  } else if (mpu.begin(ADDRESS_AD0_HIGH, &Wire)) {
    imuAddress = ADDRESS_AD0_HIGH;
  } else {
    return false;
  }

  // Match the configuration in firmware/snap_it/Inputs.cpp.
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  return true;
}

void printSample(const sensors_event_t &acceleration,
                 const sensors_event_t &rotation,
                 const sensors_event_t &temperature,
                 float accelerationMagnitudeG) {
  Serial.print(F("A[g] X="));
  Serial.print(acceleration.acceleration.x / STANDARD_GRAVITY_MPS2, 2);
  Serial.print(F(" Y="));
  Serial.print(acceleration.acceleration.y / STANDARD_GRAVITY_MPS2, 2);
  Serial.print(F(" Z="));
  Serial.print(acceleration.acceleration.z / STANDARD_GRAVITY_MPS2, 2);
  Serial.print(F(" | |A|="));
  Serial.print(accelerationMagnitudeG, 2);
  Serial.print(F(" g | G[dps] X="));
  Serial.print(rotation.gyro.x * RADIANS_TO_DEGREES, 1);
  Serial.print(F(" Y="));
  Serial.print(rotation.gyro.y * RADIANS_TO_DEGREES, 1);
  Serial.print(F(" Z="));
  Serial.print(rotation.gyro.z * RADIANS_TO_DEGREES, 1);
  Serial.print(F(" | T="));
  Serial.print(temperature.temperature, 1);
  Serial.println(F(" C"));
}

void detectShake(float accelerationMagnitudeG, uint32_t nowMs) {
  const float deltaG = fabsf(accelerationMagnitudeG - 1.0F);
  if (deltaG < SHAKE_DELTA_G ||
      static_cast<int32_t>(nowMs - nextShakeAllowedAtMs) < 0) {
    return;
  }

  ++shakeCount;
  nextShakeAllowedAtMs = nowMs + SHAKE_COOLDOWN_MS;
  Serial.print(F("*** SHAKE #"));
  Serial.print(shakeCount);
  Serial.print(F(" | |A|="));
  Serial.print(accelerationMagnitudeG, 2);
  Serial.print(F(" g | delta="));
  Serial.print(deltaG, 2);
  Serial.println(F(" g ***"));
}

}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(250);

  Serial.println();
  Serial.println(F("Adafruit STEMMA QT MPU-6050 motion and Shake test"));
  Serial.println(F("SDA GPIO1 | SCL GPIO2 | default address 0x68"));
  Serial.println(F("AD0 and INT are not used by this polling test."));

  if (!Wire.begin(PIN_IMU_SDA, PIN_IMU_SCL, I2C_FREQUENCY_HZ)) {
    Serial.println(F("ERROR: ESP32-S3 I2C initialization failed."));
    return;
  }

  scanI2cBus();
  imuReady = beginMpu6050();
  if (!imuReady) {
    Serial.println(F("ERROR: Adafruit MPU-6050 not found at 0x68 or 0x69."));
    Serial.println(F("Check VIN, GND, SDA/SCL, and common ground."));
    return;
  }

  Serial.print(F("MPU-6050 ready at 0x"));
  Serial.println(imuAddress, HEX);
  Serial.println(F("Range: +/-8 g | +/-500 dps | bandwidth: 21 Hz"));
  Serial.println(F("Keep it still, rotate each axis, then shake it."));

  const uint32_t nowMs = millis();
  nextSampleAtMs = nowMs;
  nextPrintAtMs = nowMs;
  nextShakeAllowedAtMs = nowMs + 250;
}

void loop() {
  const uint32_t nowMs = millis();
  if (!imuReady) {
    if (static_cast<int32_t>(nowMs - nextFaultMessageAtMs) >= 0) {
      Serial.println(F("IMU test inactive; fix wiring and press RESET."));
      nextFaultMessageAtMs = nowMs + 2000;
    }
    delay(10);
    return;
  }

  if (static_cast<int32_t>(nowMs - nextSampleAtMs) < 0) {
    return;
  }
  nextSampleAtMs = nowMs + SENSOR_SAMPLE_INTERVAL_MS;

  sensors_event_t acceleration;
  sensors_event_t rotation;
  sensors_event_t temperature;
  if (!mpu.getEvent(&acceleration, &rotation, &temperature)) {
    Serial.println(F("ERROR: MPU-6050 sample read failed."));
    imuReady = false;
    return;
  }

  const float accelerationMagnitudeMps2 = sqrtf(
      acceleration.acceleration.x * acceleration.acceleration.x +
      acceleration.acceleration.y * acceleration.acceleration.y +
      acceleration.acceleration.z * acceleration.acceleration.z);
  const float accelerationMagnitudeG =
      accelerationMagnitudeMps2 / STANDARD_GRAVITY_MPS2;

  detectShake(accelerationMagnitudeG, nowMs);
  if (static_cast<int32_t>(nowMs - nextPrintAtMs) >= 0) {
    nextPrintAtMs = nowMs + SERIAL_PRINT_INTERVAL_MS;
    printSample(acceleration, rotation, temperature, accelerationMagnitudeG);
  }
}
