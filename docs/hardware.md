# Hardware

## Selected prototype parts

| Function | Part |
|---|---|
| Controller | ESP32 with PSRAM; exact module is pending course approval |
| Display and microSD | Adafruit 2050, 3.5-inch 480×320 HX8357D TFT breakout |
| Focus input | Bourns PEC11R-4215F-S0024, 24-pulse quadrature encoder |
| Shake input | Adafruit 3886 MPU-6050 I²C breakout |
| Speech amplifier | Adafruit 3006 MAX98357A I²S mono amplifier |
| Speaker | PUI Audio AS02808MR-R, 8 Ω, 1 W |
| Buttons | E-Switch TL1100EF160Q and Same Sky TS02-66-70-BK-160-SCR-D |

## Provisional GPIO map

This map exists to make the development sketch buildable and breadboard-ready.
Hardware must approve it before schematic capture or PCB layout.

| Function | ESP32 GPIO |
|---|---:|
| Shared SPI SCK / MISO / MOSI | 18 / 19 / 23 |
| TFT CS / D-C / RESET | 4 / 16 / 17 |
| microSD CS | 13 |
| MPU-6050 SDA / SCL | 21 / 22 |
| MAX98357A BCLK / LRC / DIN | 26 / 25 / 27 |
| Snap / Start / Image Cycle buttons | 32 / 33 / 14 |
| Encoder A / B | 34 / 35 |

### Electrical notes

- GPIO 34 and 35 are input-only and do not provide internal pull-ups. Add
  external pull-up resistors for both encoder channels.
- Configure the display for SPI mode as described in Adafruit's HX8357D guide.
  The TFT and its microSD socket share SCK, MISO, and MOSI but use separate CS
  signals.
- Power the MAX98357A from 3.3 V for the first prototype. Its two speaker
  outputs are bridge-tied: neither speaker terminal connects to ground.
- The final ESP32 choice must expose PSRAM. Firmware refuses to start a round
  without it because the full RGB565 reveal buffer is about 270 KiB.
- Confirm supply and regulator headroom for the TFT backlight, ESP32 peak
  current, SD card, IMU, and audio amplifier before PCB layout.
- The encoder's built-in push switch is unused in the current game design.

