# SNAP button and LED bring-up

This procedure tests the E-Switch TL1100EF160Q SNAP button and a separate
indicator LED on the ESP32-S3 DevKitC-1. The display may remain connected.

## Why the LED is separate

The TL1100EF160Q is a momentary, normally-open SPST switch; it does not contain
an LED. Keep the switch input and LED output on separate GPIOs so the LED does
not change the button's logic voltage. Firmware can then debounce one press and
produce a consistent SNAP flash.

## Parts

- E-Switch TL1100EF160Q tactile switch
- One LED
- One 330-ohm resistor (220 ohms to 1 kilohm is also suitable at 3.3 V)
- Breadboard and jumper wires

## Wiring

The display test occupies GPIO 8 through GPIO 14. This test uses GPIO 5 and
GPIO 6, which do not conflict with that display wiring.

### Button input

```text
ESP32-S3 GPIO 5 ---- TL1100 switch ---- ESP32-S3 GND
```

The sketch configures GPIO 5 as `INPUT_PULLUP`, so no external pull-up resistor
is required. The input reads HIGH while released and LOW while pressed.

The switch has four physical legs but only two electrical sides. The two legs
on one side are permanently connected; pressing the actuator connects that
side to the two legs on the opposite side. Use one leg from each electrical
side. With power disconnected, a continuity meter should show:

- open circuit between the chosen legs while released;
- continuity between them while pressed.

If the input always appears pressed, the two wires are probably attached to
legs from the same electrical side. Rotate the switch 90 degrees or verify the
pairs with a continuity meter.

### LED output

```text
ESP32-S3 GPIO 6 ---- 330 ohm resistor ---- LED anode (+)
LED cathode (-) --------------------------- ESP32-S3 GND
```

The LED's longer leg is normally the anode. The cathode is normally the shorter
leg and is next to the flat edge of the LED body. The resistor may be placed on
either side of the LED, but it must be in series with it.

Do not connect an LED directly between a GPIO and ground without a resistor.

## Build and upload

From the repository root:

```sh
arduino-cli compile --profile esp32s3_button_test firmware/tests/button_test
arduino-cli board list
arduino-cli upload --profile esp32s3_button_test -p /dev/cu.YOUR_PORT firmware/tests/button_test
```

Replace `/dev/cu.YOUR_PORT` with the ESP32-S3 port. Open Serial Monitor at
115200 baud and reset the board.

## Expected result

Each physical press should:

1. print one `SNAP press` line with an incrementing count;
2. turn the external LED on for 150 ms; and
3. print one `SNAP release` line when released.

Holding the button does not repeatedly trigger presses. Contact bounce should
not create duplicate press messages.

The display breakout can remain powered and wired during this test, but this
sketch does not initialize or modify the display.

## Moving to a Feather later

The active-low button circuit and LED circuit stay the same, but the GPIO
numbers and SPI pin assignments may change with the exact Feather model. Update
the pin constants in each sketch only after selecting the Feather part number;
do not assume the DevKitC-1 header map matches a Feather header or FeatherWing.
