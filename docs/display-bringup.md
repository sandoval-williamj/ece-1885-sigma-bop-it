# Display bring-up

This procedure tests the Adafruit 2050 HX8357D TFT and its onboard microSD
socket without the rest of the game hardware. The test does not require PSRAM,
the MPU-6050, buttons, encoder, amplifier, or speaker.

## 1. Put the display in SPI mode

Disconnect power. On the Adafruit 2050 resistive-touch breakout, close the
jumper marked `IM2`/`J12` for SPI operation. If the board revision instead has
an SPI/8-bit selector, follow the silkscreen and Adafruit guide for that exact
revision. Use the header on the side marked **SPI Interface**.

## 2. Wire the ESP32-S3 DevKitC-1 to the display

The names below are the labels printed on the display. `CCS`, `Card CS`, and
`SDCS` all mean the microSD chip-select pin.

| ESP32-S3 DevKitC-1 | Adafruit 2050 | Purpose |
|---|---|---|
| `5V`/`VIN` (USB 5 V) | `3-5V`/`Vin` | Display and backlight power |
| `GND` | `GND` | Common power and signal ground |
| GPIO 12 | `CLK`/`SCK` | Shared SPI clock |
| GPIO 13 | `MISO` | Data from the microSD card |
| GPIO 11 | `MOSI` | Data to the TFT and microSD card |
| GPIO 10 | `CS`/`TCS` | TFT chip select |
| GPIO 9 | `D/C`/`DC` | TFT data/command select |
| GPIO 14 | `RST` | TFT reset |
| GPIO 8 | `CCS`/`Card CS`/`SDCS` | microSD chip select |

Do not connect the display's `3.3V out` pin to ESP32 power; it is an output
from the display breakout. `Lite` can remain disconnected because the breakout
pulls it high. Leave the touchscreen and parallel-interface pins disconnected.

## 3. Prepare the microSD card

1. Format a microSD card as FAT32.
2. Copy the `IMAGES` directory from `assets/sdcard/` to the root of the card.
3. Confirm that the card contains `/IMAGES/IMG01.BMP`.
4. With power disconnected, insert the card into the TFT breakout.

## 4. Build and upload the test

With Arduino CLI 1.4.1 installed, run from the repository root:

```sh
arduino-cli compile --profile esp32s3_display_test firmware/tests/display_test
arduino-cli board list
arduino-cli upload --profile esp32s3_display_test -p /dev/cu.YOUR_PORT firmware/tests/display_test
```

Replace `/dev/cu.YOUR_PORT` with the port shown by `arduino-cli board list`.
Alternatively, open `firmware/tests/display_test/display_test.ino` in Arduino IDE,
select **ESP32S3 Dev Module**, install the libraries listed in its `sketch.yaml`,
select the ESP32 serial port, and click Upload.

## 5. Expected result

Open Serial Monitor at 115200 baud and reset the ESP32. The screen should:

1. flash red, green, and blue;
2. show `DISPLAY OK` while checking the card; and
3. show `IMG01.BMP` below a short white status line.

If the color flashes work but the image does not, the TFT wiring is working;
check the card format, directory layout, GPIO 13/MISO, and GPIO 8/Card CS.
If the backlight is on but no colors or text appear, recheck SPI mode, GPIO 12,
GPIO 11, GPIO 10, GPIO 9, and GPIO 14.
