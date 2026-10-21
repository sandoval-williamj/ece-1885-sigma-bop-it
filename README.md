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
- `assets/sdcard/` — the exact directory tree to copy to a FAT32 microSD card.
- `docs/hardware.md` — selected parts, provisional wiring, and electrical notes.
- `docs/testing.md` — bench test and gameplay acceptance checklist.

The firmware is intentionally a conventional Arduino sketch. Open
`firmware/snap_it/snap_it.ino` in Arduino IDE, or build it reproducibly with
Arduino CLI.

## Build with Arduino CLI

Install Arduino CLI 1.4.1, then run from the repository root:

```sh
arduino-cli compile --profile esp32_psram firmware/snap_it
```

The sketch profile downloads its pinned ESP32 core and library dependencies in
an isolated build environment. Version 1.4.1 is pinned because Arduino CLI
1.5.1 has a library-discovery regression. To upload, replace the example port
with the one reported by `arduino-cli board list`:

```sh
arduino-cli upload --profile esp32_psram -p /dev/cu.usbserial-0001 firmware/snap_it
```

## Build with Arduino IDE

1. Install the ESP32 board package and select **ESP32 Dev Module**.
2. Enable PSRAM in the board options.
3. Install the library versions listed in `firmware/snap_it/sketch.yaml`.
4. Open `firmware/snap_it/snap_it.ino`, compile, and upload.
5. Copy the contents of `assets/sdcard/` to the root of a FAT32 microSD card.

The GPIO assignments are provisional. Hardware must review `Config.h` and
`docs/hardware.md` before PCB layout.

## Team workflow

Keep game rules in `Game`, physical input detection in `Inputs`, screen/image
work in `Display`, and speech playback in `Audio`. Put tunable thresholds,
timings, pins, and the ordered image manifest in `Config.h`. Avoid adding a new
layer or dependency unless it removes more complexity than it introduces.

