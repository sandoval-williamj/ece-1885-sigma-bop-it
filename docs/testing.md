# Test checklist

Record the board revision, firmware commit, tester, and result for each bench
session. Use the Serial Monitor at 115200 baud alongside the display.

## Build and startup

- [ ] `arduino-cli compile --profile esp32s3_embedded firmware/snap_it`
      succeeds and remains below the 3 MB application limit.
- [ ] The board initializes the TFT, MPU-6050, and I²S audio without entering
      the red hardware-fault screen.
- [ ] The game starts and completes a round with no microSD card installed and
      with PSRAM disabled.
- [ ] With `SERIAL_ONLY_MODE=true` and the TFT disconnected, Serial Monitor
      shows selection, command, score, reveal, game-over, and victory output.

## Component tests

- [ ] Image Cycle advances through all labels and wraps to the first.
- [ ] Start begins a round; Image Cycle and Start do nothing during the round.
- [ ] Snap registers once per press without contact-bounce duplicates.
- [ ] Every debounced SNAP press pulses the GPIO6 indicator LED for 150 ms,
      including an incorrect SNAP response.
- [ ] One encoder detent in either direction registers Focus once.
- [ ] Normal handling does not register Shake; a deliberate shake registers
      once and respects the cooldown.
- [ ] Embedded SNAP, FOCUS, and SHAKE speech is intelligible and free of
      dropouts.
- [ ] The MAX98357A and speaker remain within safe temperature and volume.

## Gameplay tests

- [ ] Commands are independently random and may repeat immediately.
- [ ] The response timer starts after the spoken prompt finishes.
- [ ] The timer begins at 5 seconds, decreases by 40 ms per point, and never
      drops below 1 second.
- [ ] A correct action reveals exactly 10 previously hidden tiles.
- [ ] A wrong first action or timeout ends the round immediately.
- [ ] A loss preserves the partial image and final score.
- [ ] At score 99 all 990 tiles are visible and victory is shown.
- [ ] After a completed round, Start replays the selected image and Image Cycle
      returns to selection with the next label.

