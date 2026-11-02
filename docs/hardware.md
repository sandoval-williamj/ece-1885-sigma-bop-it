# Hardware

## Selected prototype parts

| Function | Part |
|---|---|
| Controller | ESP32-S3 DevKitC-1 for current bench testing |
| Display and microSD | Adafruit 2050, 3.5-inch 480×320 HX8357D TFT breakout |
| Focus input | Bourns PEC11R-4215F-S0024, 24-pulse quadrature encoder |
| Shake input | Adafruit 3886 MPU-6050 I²C breakout |
| Speech amplifier | Adafruit 3006 MAX98357A I²S mono amplifier |
| Speaker | Current bench unit: 8 Ω, 25 W; exact part number not recorded |
| Buttons | E-Switch TL1100EF160Q and Same Sky TS02-66-70-BK-160-SCR-D |

## Provisional GPIO map

This map exists to make the development sketch buildable and breadboard-ready.
Hardware must approve it before schematic capture or PCB layout.

| Function | ESP32 GPIO |
|---|---:|
| Shared SPI SCK / MISO / MOSI | 12 / 13 / 11 |
| TFT CS / D-C / RESET | 10 / 9 / 14 |
| microSD CS | 8 |
| SNAP button / indicator LED | 5 / 6 |
| Focus encoder A / B | 15 / 16 |
| Passive test buzzer | 17 |
| MAX98357A BCLK / LRC / DIN | 39 / 40 / 41 |
| Adafruit MPU-6050 SDA / SCL | 1 / 2 (reserved; not yet tested) |
| Remaining game buttons | Not assigned on the ESP32-S3 bench build |

### Electrical notes

- The current table records the working ESP32-S3 DevKitC-1 bench wiring. A
  later Feather migration will require an explicit pin-map review.
- Configure the display for SPI mode as described in Adafruit's HX8357D guide.
  The TFT and its microSD socket share SCK, MISO, and MOSI but use separate CS
  signals.
- Power the MAX98357A from a regulated 5 V supply with adequate current
  headroom. Its two speaker outputs are bridge-tied: neither speaker terminal
  connects to ground.
- The final ESP32 choice must expose PSRAM. Firmware refuses to start a round
  without it because the full RGB565 reveal buffer is about 270 KiB.
- Confirm supply and regulator headroom for the TFT backlight, ESP32 peak
  current, SD card, IMU, and audio amplifier before PCB layout.
- The encoder's built-in push switch is unused in the current game design.

