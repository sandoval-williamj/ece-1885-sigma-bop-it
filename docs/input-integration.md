# Display and input integration test

This test runs the Adafruit 2050 display, SNAP button and LED, and Focus rotary
encoder together on the ESP32-S3 DevKitC-1. It does not initialize or require a
microSD card.

## Wiring

Keep the working component-test wiring:

| Function | ESP32-S3 GPIO |
|---|---:|
| SNAP button to ground | 5 |
| SNAP indicator LED through 330 ohms to ground | 6 |
| Display Card CS, left inactive | 8 |
| Display D/C | 9 |
| Display TFT CS | 10 |
| Display MOSI | 11 |
| Display SCK | 12 |
| Display MISO, unused by this test | 13 |
| Display RESET | 14 |
| Encoder A | 15 |
| Encoder B | 16 |

Connect encoder C/common and the button's other terminal to ESP32 ground. All
components must share that common ground.

The display's Card CS pin is driven HIGH so the untested microSD interface
cannot interfere with the shared SPI bus.

## Build and upload

From the repository root:

```sh
arduino-cli compile --profile esp32s3_input_integration_test firmware/tests/input_integration_test
arduino-cli board list
arduino-cli upload --profile esp32s3_input_integration_test -p /dev/cu.YOUR_PORT firmware/tests/input_integration_test
```

Open Serial Monitor at 115200 baud after uploading.

## Expected result

The display should show an `INPUT INTEGRATION` dashboard.

At startup it first flashes red, green, and blue for 300 ms each. Serial
Monitor prints a matching marker for every color and then
`TFT startup: dashboard drawn`. The TFT runs at a conservative 8 MHz in this
combined breadboard test to tolerate longer jumper wiring.

- Each SNAP press increments `SNAP COUNT`, shows `SNAP`, and pulses the LED.
- Each encoder detent updates `FOCUS POSITION` and shows `FOCUS CW` or
  `FOCUS CCW`.
- Holding the SNAP button should not create repeated presses.
- Turning one full encoder revolution should change the Focus position by
  approximately 24.

The sketch uses the ESP32 hardware pulse counter through `ESP32Encoder`, like
the current game firmware, so display drawing does not block encoder edge
capture. If the displayed direction is reversed, swap encoder A and B.
