# Hardware bring-up tests

Each directory is a standalone Arduino sketch for one hardware subsystem.
Keeping the tests isolated makes it easier to distinguish a wiring problem
from an interaction between components.

| Sketch | Purpose | ESP32-S3 GPIOs |
|---|---|---|
| `screen_test` | Repeating TFT color and text test; no microSD card | 9-12, 14 |
| `display_test` | TFT plus microSD image test | 8-14 |
| `button_test` | Debounced SNAP button and separate LED pulse | 5, 6 |
| `encoder_test` | Quadrature decoding for the Focus encoder | 15, 16 |
| `input_integration_test` | TFT feedback for SNAP and Focus together | 5-6, 8-16 |
| `buzzer_test` | Repeating passive-piezo frequency test | 17 |
| `amplifier_speaker_test` | MAX98357A I2S three-tone speaker test | 39-41 |
| `imu_test` | Adafruit STEMMA QT MPU-6050 and Shake test | 1, 2 |
| `pitt_image_test` | Three-photo Pitt slideshow embedded in flash | 9-12, 14 |
| `two_input_game_test` | Two-input game with randomized pixel reveals | 5, 6, 8-12, 14-17 |

Run compile and upload commands from the repository root. Each sketch has its
own `sketch.yaml` profile and can also be opened directly in Arduino IDE.

The tests currently target the ESP32-S3 DevKitC-1. Pin constants are local to
each sketch so a future Feather migration can be reviewed explicitly.

## Expected display behavior

`screen_test` repeats five two-second states: red, green, blue, white, and
black. Each state includes a text label; the black state says `SCREEN WORKS`.

`display_test` runs once after reset. It flashes red, green, and blue for 300 ms
each, then shows `DISPLAY OK` and either the BMP image or a specific SD/image
error. It does not draw line patterns or continuously alternate colors. Those
patterns usually mean the Adafruit `graphicstest` example or a previously
uploaded sketch is still running. Check Serial Monitor at 115200 baud for
`HX8357 display bring-up test` to confirm the intended sketch started.

`pitt_image_test` embeds three real University of Pittsburgh photographs in
ESP32 flash and advances every five seconds. It does not require a microSD
card.

`two_input_game_test` combines the working display, SNAP button/LED, Focus
encoder, and optional buzzer into a Bop It-style round. It randomly selects an
embedded Pitt photograph and reveals new random pixels after every correct
input. It also does not require a microSD card.

`amplifier_speaker_test` sends three low-level sine tones to the MAX98357A over
I2S. The existing game hardware may remain connected because it uses the free
GPIO39-41 pins.

`imu_test` uses Adafruit's MPU6050 library and the same ranges and Shake
threshold as the game. It prints accelerometer, gyroscope, temperature, and
shake events on the reserved GPIO1/2 I2C pair.
