#include "Audio.h"
#include "Config.h"
#include "Display.h"
#include "Game.h"
#include "Inputs.h"

Inputs inputs;
Display display;
Audio audio;
Game game(inputs, display, audio);

void setup() {
  Serial.begin(Config::SERIAL_BAUD);
  Serial.println();
  Serial.println(F("SNAP IT! starting"));

  display.begin();
  inputs.begin();
  audio.begin();
  game.begin();
}

void loop() { game.update(millis()); }

