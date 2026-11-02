# SNAP IT!

SNAP IT! is an ESP32 camera-shaped reaction game inspired by Bop It. The game
speaks one of three commands:

- **Snap It!** — press the shutter button.
- **Focus It!** — rotate the lens encoder.
- **Shake It!** — shake the enclosure.

Each correct response reveals ten shuffled pieces of a selected image. A score
of 99 reveals all 990 pieces. A wrong action or timeout ends the round and
leaves the earned portion visible.

## Repository layout

- `firmware/snap_it/` — the Arduino sketch and its small firmware modules.
- `firmware/tests/` — isolated ESP32-S3 bring-up sketches for the display,
  buttons, and future hardware components.
- `assets/sdcard/` — source copies of the voice prompts and legacy BMP fixtures.
- `docs/hardware.md` — selected parts, provisional wiring, and electrical notes.
- `docs/display-bringup.md` — minimum wiring and upload steps for the display.
- `docs/button-bringup.md` — SNAP button and indicator LED wiring and test.
- `docs/encoder-bringup.md` — Focus rotary encoder wiring and test.
- `docs/input-integration.md` — combined display, SNAP, and Focus input test.
- `docs/buzzer-bringup.md` — optional passive-piezo PWM test.
- `docs/amplifier-speaker-bringup.md` — MAX98357A and speaker wiring/test.
- `docs/imu-bringup.md` — Adafruit STEMMA QT MPU-6050 and Shake test.
- `docs/embedded-assets.md` — production flash image/audio storage design.
- `docs/pitt-image-bringup.md` — three-photo Pitt slideshow embedded in flash.
- `docs/two-input-game-bringup.md` — two-input game and random-pixel reveal test.
- `docs/testing.md` — bench test and gameplay acceptance checklist.

The firmware is intentionally a conventional Arduino sketch. Open
`firmware/snap_it/snap_it.ino` in Arduino IDE, or build it reproducibly with
Arduino CLI.

## Build with Arduino CLI

Install Arduino CLI 1.4.1, then run from the repository root:

```sh
arduino-cli compile --profile esp32s3_embedded firmware/snap_it
```

The sketch profile downloads its pinned ESP32 core and library dependencies in
an isolated build environment. Version 1.4.1 is pinned because Arduino CLI
1.5.1 has a library-discovery regression. To upload, replace the example port
with the one reported by `arduino-cli board list`:

```sh
arduino-cli upload --profile esp32s3_embedded -p /dev/cu.usbmodem0001 firmware/snap_it
```

## Build with Arduino IDE

1. Install the ESP32 board package and select **ESP32S3 Dev Module**.
2. Select at least 4 MB flash and the **Huge APP (3 MB No OTA)** partition.
   PSRAM may remain disabled.
3. Install the library versions listed in `firmware/snap_it/sketch.yaml`.
4. Open `firmware/snap_it/snap_it.ino`, compile, and upload.

The Pitt photographs and spoken commands are compiled into the sketch. No
microSD card or media-copy step is required.

The GPIO assignments are provisional. Hardware must review `Config.h` and
`docs/hardware.md` before PCB layout.

## Team workflow

Keep game rules in `Game`, physical input detection in `Inputs`, screen/image
work in `Display`, speech playback in `Audio`, and the ordered flash media
manifest in `EmbeddedAssets`. Put tunable thresholds, timings, and pins in
`Config.h`. Avoid adding a new layer or dependency unless it removes more
complexity than it introduces.

