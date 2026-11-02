# Embedded production media

The production `snap_it` sketch is self-contained. It does not initialize a
microSD card and does not require PSRAM.

`EmbeddedAssets.cpp` exposes three flash-resident RGB565 images and three
flash-resident PCM WAV voice prompts:

| Asset | Embedded representation |
|---|---|
| Pitt campus, Heinz Chapel, Pitt Panther | 480×288 RGB565, 276,480 bytes each |
| SNAP, FOCUS, SHAKE speech | Mono 16-bit PCM WAV at 22,050 Hz |

The image payload totals 829,440 bytes and the original-quality speech totals
108,338 bytes. The ESP32-S3 build uses the 3 MB `Huge APP` partition on a 4 MB
or larger flash chip, leaving substantial application headroom. OTA updates
are disabled by that partition layout.

The original editable/source media remains in:

- `assets/pitt_test_images/` for the licensed Pitt photographs and attribution;
- `assets/sdcard/AUDIO/` for the replaceable voice WAV files;
- `assets/sdcard/IMAGES/` for the older generated BMP fixtures.

The compiled arrays live beside the sketch as `PITT01.inc` through
`PITT03.inc` and `SNAP_WAV.inc`, `FOCUS_WAV.inc`, and `SHAKE_WAV.inc`.
`EmbeddedAssets.h` is the manifest used by the display and audio modules.

The display reveals small tiles directly from flash into a temporary tile
buffer. It never allocates a complete image canvas. Audio uses
`AudioFileSourcePROGMEM` to decode each WAV directly from flash through the
MAX98357A I2S amplifier.
