# Adafruit STEMMA QT MPU-6050 bring-up

This standalone test targets the Adafruit product 3886 MPU-6050 6-DoF
accelerometer and gyroscope. It uses Adafruit's Arduino library and the same
sensor ranges, filter bandwidth, sample interval, and Shake threshold as the
current `snap_it` input code.

## Power-off wiring

Disconnect power before changing wires. The ESP32-S3 uses 3.3 V logic, so
power the breakout's `VIN` pin from 3.3 V. The Adafruit board includes its own
regulator, I2C level shifting, and SDA/SCL pull-ups.

| ESP32-S3 DevKitC-1 | Adafruit MPU-6050 |
|---|---|
| 3V3 | VIN |
| GND | GND |
| GPIO 2 | SCL |
| GPIO 1 | SDA |

Leave `3Vo`, `INT`, and `AD0` disconnected. `3Vo` is a regulated output, not
the board's normal power input. Open AD0 selects the default `0x68` address;
pulling it high changes the address to `0x69`. This polling test does not need
the interrupt pin.

If using a STEMMA QT-to-male-header cable instead of the through-hole pads:

| STEMMA QT wire | ESP32-S3 |
|---|---|
| Black | GND |
| Red | 3V3 |
| Blue | GPIO 1 / SDA |
| Yellow | GPIO 2 / SCL |

Either of the two STEMMA QT sockets can be used. GPIO1/2 are free on the
present bench build. The display/button/encoder/buzzer on GPIO5–17 and
MAX98357A amplifier on GPIO39–41 may remain connected. Every powered module
must share ESP32 ground.

## Build and upload

From the repository root:

```sh
arduino-cli compile --profile esp32s3_imu_test firmware/tests/imu_test
arduino-cli board list
arduino-cli upload --profile esp32s3_imu_test -p /dev/cu.YOUR_PORT firmware/tests/imu_test
```

Open Serial Monitor at 115200 baud and reset the ESP32-S3.

## Expected result

At startup, Serial Monitor should find an I2C device at `0x68`, identify the
MPU-6050, and report:

```text
Range: +/-8 g | +/-500 dps | bandwidth: 21 Hz
```

The sensor is sampled 50 times per second while readable output is printed ten
times per second:

```text
A[g] X=0.01 Y=-0.02 Z=0.99 | |A|=0.99 g | G[dps] X=0.4 Y=-0.2 Z=0.1 | T=25.8 C
```

Test in three stages:

1. Keep the board still. Total acceleration `|A|` should be close to `1.00 g`
   and gyro readings should be near zero.
2. Rotate the board around each axis. The corresponding gyro number should
   change strongly and then return near zero when motion stops.
3. Give it a deliberate short shake. Serial Monitor should print
   `*** SHAKE #... ***`. The test uses the game's 1.2 g deviation threshold
   and 600 ms cooldown.

Small uncalibrated zero offsets are normal. This test verifies the exact
hardware and configuration that the game will use.
