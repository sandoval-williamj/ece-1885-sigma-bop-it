# Source and legacy media

The production game no longer reads a microSD card. These files are retained
as editable/source assets for regenerating the arrays compiled into
`firmware/snap_it/`.

## Images

The `IMAGES` directory contains the original generated integration fixtures.
The production game currently embeds the licensed photographs from
`assets/pitt_test_images/`; their order and labels are defined in
`firmware/snap_it/EmbeddedAssets.cpp`.

The included images are generated test artwork, not final product graphics.
Replace them in place or update the manifest when the graphics are ready.

Example conversion with ffmpeg:

```sh
ffmpeg -i source.png -vf scale=480:288:force_original_aspect_ratio=decrease,pad=480:288:-1:-1:black -pix_fmt bgr24 IMAGES/IMG01.BMP
```

## Audio

Command prompts are mono, signed 16-bit PCM WAV files sampled at 22,050 Hz.
The included computer-generated speech is embedded unchanged in the current
firmware and remains a replaceable integration fixture.

Example conversion:

```sh
ffmpeg -i recording.wav -ac 1 -ar 22050 -c:a pcm_s16le AUDIO/SNAP.WAV
```

