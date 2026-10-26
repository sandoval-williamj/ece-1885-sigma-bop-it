# Test checklist

Record the board revision, firmware commit, tester, and result for each bench
session. Use the Serial Monitor at 115200 baud alongside the display.

## Build and startup

- [ ] `arduino-cli compile --profile esp32_psram firmware/snap_it` succeeds.
- [ ] The board reports PSRAM and initializes the TFT, SD card, MPU-6050, and
      I²S audio without entering the red hardware-fault screen.
- [ ] Removing the SD card, one manifest image, one command WAV, or the IMU
      produces a useful LCD and Serial error instead of a reset or crash.

## Component tests

- [ ] Image Cycle advances through all labels and wraps to the first.
- [ ] Start begins a round; Image Cycle and Start do nothing during the round.
- [ ] Snap registers once per press without contact-bounce duplicates.
- [ ] One encoder detent in either direction registers Focus once.
- [ ] Normal handling does not register Shake; a deliberate shake registers
      once and respects the cooldown.
- [ ] SNAP, FOCUS, and SHAKE WAV files are intelligible and free of dropouts.
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

