#pragma once

#include <Arduino.h>

#include "GameTypes.h"

class Audio;
class Display;
class Inputs;

class Game {
 public:
  Game(Inputs &inputs, Display &display, Audio &audio);

  void begin();
  void update(uint32_t nowMs);
  GameState state() const { return state_; }

 private:
  void startRound(uint32_t nowMs);
  void startPrompt();
  void handleCorrect(uint32_t nowMs);
  void finishRound(bool timedOut);
  void enterFault(const char *message);
  uint32_t responseWindowMs() const;

  Inputs &inputs_;
  Display &display_;
  Audio &audio_;
  GameState state_ = GameState::HardwareFault;
  Command expectedCommand_ = Command::Snap;
  size_t selectedImage_ = 0;
  uint8_t score_ = 0;
  uint32_t responseStartedMs_ = 0;
  uint32_t feedbackUntilMs_ = 0;
};

