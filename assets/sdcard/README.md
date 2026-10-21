# SD card media

Copy the `IMAGES` and `AUDIO` directories to the root of a FAT32 microSD card.
The filenames deliberately use the conservative 8.3 format.

## Images

Firmware images must be uncompressed, 24-bit, 480×288 BMP files. Their order
and user-facing labels are defined in `firmware/snap_it/Config.h`.

The included images are generated test artwork, not final product graphics.
Replace them in place or update the manifest when the graphics are ready.

Example conversion with ffmpeg:

```sh
ffmpeg -i source.png -vf scale=480:288:force_original_aspect_ratio=decrease,pad=480:288:-1:-1:black -pix_fmt bgr24 IMAGES/IMG01.BMP
```

## Audio

Command prompts are mono, signed 16-bit PCM WAV files sampled at 22,050 Hz.
The included computer-generated speech is a replaceable integration fixture.

Example conversion:

```sh
ffmpeg -i recording.wav -ac 1 -ar 22050 -c:a pcm_s16le AUDIO/SNAP.WAV
```

