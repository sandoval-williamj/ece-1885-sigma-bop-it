# Passive piezo buzzer bring-up

This optional test drives the Mallory Sonalert PT-1520PQ from the ESP32-S3.
The part is an externally driven piezo transducer, not an active DC buzzer. It
needs a square wave and is loudest near its 2 kHz resonant frequency.

This test is useful for validating PWM and providing temporary audible
feedback while the game speaker is unavailable. The buzzer cannot reproduce
the game's spoken WAV commands and should not be connected to the MAX98357
speaker output.

## Wiring

```text
ESP32-S3 GPIO 17 ---- 100 to 220 ohm resistor ---- buzzer +
ESP32-S3 GND ------------------------------------- buzzer -
```

GPIO 17 is unused by the current ESP32-S3 component-test wiring. The small
series resistor limits edge current into the capacitive piezo element. Do not
connect the buzzer directly to 5 V.

## Build and upload

```sh
arduino-cli compile --profile esp32s3_buzzer_test firmware/tests/buzzer_test
arduino-cli board list
arduino-cli upload --profile esp32s3_buzzer_test -p /dev/cu.YOUR_PORT firmware/tests/buzzer_test
```

Open Serial Monitor at 115200 baud.

## Expected result

The sketch repeatedly plays 1500 Hz, 2000 Hz, and 2500 Hz for 350 ms each,
with short gaps and then a three-second pause. The 2000 Hz tone should normally
sound strongest because it is closest to the specified resonance.

The test uses an approximately 3.3 V peak-to-peak, 50 percent duty-cycle square
wave. Stop the test by resetting, disconnecting, or uploading another sketch.
