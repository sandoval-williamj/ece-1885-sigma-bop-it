# MAX98357A amplifier and speaker test

This test drives an 8-ohm moving-coil speaker from the Adafruit 3006
MAX98357A mono I2S amplifier. It generates three low-level sine tones directly
on the ESP32-S3, so it requires no display, microSD card, WAV file, or external
Arduino library.

The speaker's 25 W rating is its power-handling capacity, not the power it
draws. The MAX98357A can produce only about 1.8 W into 8 ohms from a 5 V
supply, so the combination is electrically compatible but will not use the
speaker's full loudness capability.

## Power-off wiring

Disconnect power while attaching the speaker and amplifier. The selected I2S
pins are exposed, unused pins on the ESP32-S3 DevKitC-1 and do not conflict
with the working display, SNAP button/LED, encoder, or buzzer wiring.

| ESP32-S3 / supply | MAX98357A |
|---|---|
| GPIO 39 | BCLK |
| GPIO 40 | LRC |
| GPIO 41 | DIN |
| Regulated 5 V | Vin |
| Common ground | GND |

Then connect the speaker directly across the amplifier output:

| Speaker | MAX98357A screw terminal |
|---|---|
| One speaker lead | `+` |
| Other speaker lead | `-` |

The `+` and `-` outputs are a bridge-tied pair. **Do not connect either
speaker lead or either amplifier output to ground.** Speaker polarity will not
affect this single-speaker component test, but keeping `+` and `-` consistent
will matter if another speaker is added later.

For the Adafruit 3006 board powered from 5 V:

- Leave `GAIN` disconnected for its default 9 dB gain.
- Leave `SD` disconnected; the breakout's onboard resistor selects its
  default mono mix.
- Leave MCLK disconnected; the MAX98357A does not use it.

Use a regulated 5 V source capable of at least 800 mA for the amplifier. If
the same source also powers the display, allow additional current headroom;
1.5–2 A is a practical bench target. The safest shared-supply arrangement is
to keep the ESP32-S3 powered by USB, power the display/amplifier from the
external regulated 5 V supply, and join all grounds. Do not join two separate
5 V source outputs together.

## Build and upload

From the repository root:

```sh
arduino-cli compile --profile esp32s3_amplifier_speaker_test firmware/tests/amplifier_speaker_test
arduino-cli board list
arduino-cli upload --profile esp32s3_amplifier_speaker_test -p /dev/cu.YOUR_PORT firmware/tests/amplifier_speaker_test
```

Open Serial Monitor at 115200 baud and reset the ESP32-S3.

## Expected result

The speaker repeatedly plays 440 Hz, 660 Hz, and 880 Hz sine tones followed by
three seconds of silence. Serial Monitor prints each tone as it begins.

The test starts at only 8% of digital full scale and applies short fades to
avoid harsh clicks. The three tones should be clean and clearly different in
pitch. Stop and remove power if the amplifier or speaker becomes hot, the
speaker cone moves violently, or the sound is heavily distorted.

If Serial reports normal playback but no tone is audible, check common ground,
confirm that GPIO39 goes to `BCLK` and GPIO40 goes to `LRC` (not reversed), and
verify that neither speaker terminal is grounded.
