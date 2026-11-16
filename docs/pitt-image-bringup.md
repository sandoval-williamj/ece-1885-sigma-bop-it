# University of Pittsburgh flash-photo slideshow

This test displays three real University of Pittsburgh photographs embedded
directly in the ESP32-S3 firmware. It does not initialize or require a microSD
card.

The photographs show:

1. the University of Pittsburgh campus viewed from the Cathedral of Learning;
2. Heinz Memorial Chapel; and
3. the Pitt Panther statue.

Source, modification, and license details are recorded in
`assets/pitt_test_images/ATTRIBUTION.md`. Preview BMPs are stored beside that
file; they do not need to be copied anywhere for this test.

## How the images fit

Three uncompressed 24-bit 480×288 BMPs would occupy about 1.24 MB before adding
firmware. The test instead stores each image as 16-bit RGB565, reducing the
total embedded image payload to 829,440 bytes. The complete compiled sketch
fits in the default 1,310,720-byte ESP32-S3 application partition.

The images are compiled into `PITT01.inc`, `PITT02.inc`, and `PITT03.inc` in the
test sketch directory. No filesystem or external storage is involved.

## Wiring

Only the TFT-side SPI signals are required:

| ESP32-S3 | Adafruit 2050 |
|---|---|
| 5V | 3-5V/Vin |
| GND | GND |
| GPIO 12 | CLK/SCK |
| GPIO 11 | MOSI |
| GPIO 10 | CS/TCS |
| GPIO 9 | D/C |
| GPIO 14 | RST |

MISO and Card CS are not used. The ESP32, display, and any external 5 V supply
must share a common ground.

## Build and upload

```sh
arduino-cli compile --profile esp32s3_pitt_image_test firmware/tests/pitt_image_test
arduino-cli board list
arduino-cli upload --profile esp32s3_pitt_image_test -p /dev/cu.YOUR_PORT firmware/tests/pitt_image_test
```

Open Serial Monitor at 115200 baud and reset the ESP32-S3.

## Expected result

The screen shows a blue title bar and cycles through the three photographs
every five seconds. Serial Monitor reports the label of each image as it is
drawn.

If the screen remains blank, confirm the same TFT wiring with `screen_test`,
particularly GPIO 9/D-C, GPIO 10/CS, GPIO 11/MOSI, GPIO 12/CLK, GPIO 14/RESET,
and common ground.
