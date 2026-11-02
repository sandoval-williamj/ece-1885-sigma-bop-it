# Focus rotary encoder bring-up

This procedure tests rotation of the Bourns PEC11R-4215F-S0024 incremental
encoder on the ESP32-S3 DevKitC-1. The display and SNAP button may remain
connected.

The selected encoder has 24 detents and produces 24 quadrature pulses per full
rotation. It also contains a momentary shaft push switch, but that switch is
not used by the current game design.

## Identify the terminals

The three encoder terminals are labeled `A`, `C`, and `B` in the Bourns data
sheet:

- `A` is one quadrature channel;
- `C` is the common contact; and
- `B` is the other quadrature channel.

The two separate terminals labeled `1` and `2` belong to the unused shaft push
switch. Leave them disconnected for this test.

Use the labels or the data-sheet drawing rather than assuming left and right
from a photograph; viewing the part from the opposite side reverses the order.
With power disconnected, a continuity meter can also identify `C`: rotation
causes it to connect intermittently to both `A` and `B`.

## Wiring

```text
Bourns terminal A ---- ESP32-S3 GPIO 15
Bourns terminal C ---- ESP32-S3 GND
Bourns terminal B ---- ESP32-S3 GPIO 16
```

The sketch uses `INPUT_PULLUP` on A and B, so the encoder needs no power wire
and no external pull-up resistors for this short-wire breadboard test. It is a
passive switch-contact encoder; do not connect C to 3.3 V.

This pin selection does not conflict with the current test wiring:

- display: GPIO 8 through GPIO 14;
- SNAP button: GPIO 5;
- SNAP LED: GPIO 6; and
- Focus encoder: GPIO 15 and GPIO 16.

Keep the encoder wires short. If the final enclosure wiring is long or the
test reports noisy movement, add 10-kilohm pull-up resistors from A and B to
3.3 V and review the filter circuit in the Bourns data sheet.

## Build and upload

From the repository root:

```sh
arduino-cli compile --profile esp32s3_encoder_test firmware/tests/encoder_test
arduino-cli board list
arduino-cli upload --profile esp32s3_encoder_test -p /dev/cu.YOUR_PORT firmware/tests/encoder_test
```

Replace `/dev/cu.YOUR_PORT` with the ESP32-S3 port. Open Serial Monitor at
115200 baud and reset the board.

## Expected result

Rotate the shaft slowly one detent at a time. Each detent should print one line
and change the position by one:

```text
CW detent | position: 1
CW detent | position: 2
CCW detent | position: 1
```

One complete revolution should change the reported position by approximately
24. Holding the shaft still should produce no messages.

If clockwise rotation prints `CCW`, swap the A and B wires; direction depends
on which side of the component is used as the viewing reference. If counts
jump, reverse unexpectedly, or generate invalid-transition messages, check the
common connection and shorten or replace the jumper wires.

The test decodes all four quadrature edges for each pulse and reports only a
completed detent. It does not use delays, so quick rotations are less likely to
be missed.

## microSD requirement for SNAP IT

The ESP32-S3 DevKitC-1 does not need its own microSD socket. The Adafruit 2050
display breakout already contains the microSD socket, and the ESP32 accesses
that socket over the shared SPI bus using GPIO 8 for Card CS.

The current game firmware requires the card. It loads the reveal images and
the spoken-command WAV files from `assets/sdcard/`, and it enters a hardware
fault if the card cannot be initialized. Removing this requirement would need
a firmware and storage redesign to place the assets in onboard flash. PSRAM
cannot replace the card because PSRAM is temporary working memory, not
persistent storage.
