# Two-input SNAP IT game test

This standalone test combines the hardware that has passed bring-up so far:
the Adafruit 2050 display, SNAP button and LED, Focus rotary encoder, and the
optional passive piezo buzzer. It uses only two gameplay inputs—SNAP and
Focus—and does not initialize or require a microSD card, IMU, I2S amplifier,
or speaker.

The three University of Pittsburgh photographs are compiled into ESP32-S3
flash as RGB565 pixel arrays. Their sources and licenses are recorded in
`assets/pitt_test_images/ATTRIBUTION.md`.

## Wiring

All grounds must be connected, including the ESP32-S3, display, encoder,
button/LED circuit, buzzer, and any external 5 V supply.

| Component signal | ESP32-S3 connection |
|---|---:|
| SNAP button, other terminal to GND | GPIO 5 |
| SNAP LED anode through 330 ohms, cathode to GND | GPIO 6 |
| Display Card CS, held inactive | GPIO 8 |
| Display D/C | GPIO 9 |
| Display TFT CS | GPIO 10 |
| Display MOSI | GPIO 11 |
| Display SCK/CLK | GPIO 12 |
| Display RESET | GPIO 14 |
| Encoder A | GPIO 15 |
| Encoder B | GPIO 16 |
| Encoder C/common | GND |
| PT-1520PQ `+` through 100–220 ohms | GPIO 17 |
| PT-1520PQ `-` | GND |

Power the display through its `3-5V/Vin` pin. Display MISO is not used. The
buzzer is optional; leaving GPIO 17 disconnected only removes the tones.

## Build and upload

From the repository root:

```sh
arduino-cli compile --profile esp32s3_two_input_game_test firmware/tests/two_input_game_test
arduino-cli board list
arduino-cli upload --profile esp32s3_two_input_game_test -p /dev/cu.YOUR_PORT firmware/tests/two_input_game_test
```

Open Serial Monitor at 115200 baud after uploading.

## Expected gameplay

The game starts automatically after a short title screen. Each round randomly
selects one of the three embedded photographs and starts with its image area
black. The top bar asks for one of two actions:

- `SNAP IT!`: press the SNAP button.
- `FOCUS IT!`: rotate the encoder at least one detent in either direction.

A correct action reveals 1,397 previously hidden pixels at randomized
positions, flashes `NICE!`, and then issues another random command. Pixels are
never repeated within a round. At score 99 the final pixels are revealed and
the complete photograph is visible.

The response window begins at five seconds and decreases by 40 ms after each
point, down to a one-second minimum. A wrong input or timeout ends the round
and leaves the partially revealed image visible. Press SNAP to start a new
round with a newly selected photograph and new pixel order.

Prompt, success, failure, and victory tones are simple piezo cues; the command
text on the display is authoritative. Serial Monitor also prints every
command, result, selected image, and reveal progress.
