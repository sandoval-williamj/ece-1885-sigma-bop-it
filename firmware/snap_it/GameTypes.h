#pragma once

#include <Arduino.h>

enum class Command : uint8_t {
  Snap = 0,
  Focus = 1,
  Shake = 2,
};

enum class GameState : uint8_t {
  Selecting,
  Prompting,
  AwaitingInput,
  SuccessFeedback,
  GameOver,
  Victory,
  HardwareFault,
};

inline const __FlashStringHelper *commandLabel(Command command) {
  switch (command) {
    case Command::Snap:
      return F("SNAP IT!");
    case Command::Focus:
      return F("FOCUS IT!");
    case Command::Shake:
      return F("SHAKE IT!");
  }
  return F("UNKNOWN");
}

