#include "Game.h"

#include <esp_system.h>

#include "Audio.h"
#include "Config.h"
#include "Display.h"
#include "Inputs.h"

Game::Game(Inputs &inputs, Display &display, Audio &audio)
    : inputs_(inputs), display_(display), audio_(audio) {}

void Game::begin() {
  if (!display_.isReady()) {
    enterFault(display_.lastError());
    return;
  }
  if (!inputs_.isReady()) {
    enterFault("Required game input hardware is unavailable");
    return;
  }
  if (!audio_.isReady()) {
    enterFault(audio_.lastError());
    return;
  }

  state_ = GameState::Selecting;
  display_.showPicker(selectedImage_);
}

void Game::update(uint32_t nowMs) {
  inputs_.update(nowMs);
  audio_.update();

  switch (state_) {
    case GameState::Selecting:
    case GameState::GameOver:
    case GameState::Victory:
      if (inputs_.takeImagePress()) {
        selectedImage_ = (selectedImage_ + 1) % Config::IMAGE_COUNT;
        state_ = GameState::Selecting;
        display_.showPicker(selectedImage_);
      } else if (inputs_.takeStartPress()) {
        startRound(nowMs);
      }
      break;

    case GameState::Prompting:
      if (!audio_.isPlaying()) {
        inputs_.resetGameplayEvents(nowMs);
        responseStartedMs_ = nowMs;
        state_ = GameState::AwaitingInput;
      }
      break;

    case GameState::AwaitingInput: {
      Command received;
      if (inputs_.takeGameplayAction(received)) {
        if (received == expectedCommand_) {
          handleCorrect(nowMs);
        } else {
          finishRound(false);
        }
      } else if (nowMs - responseStartedMs_ >= responseWindowMs()) {
        finishRound(true);
      }
      break;
    }

    case GameState::SuccessFeedback:
      if (static_cast<int32_t>(nowMs - feedbackUntilMs_) >= 0) {
        startPrompt();
      }
      break;

    case GameState::HardwareFault:
      break;
  }
}

void Game::startRound(uint32_t nowMs) {
  score_ = 0;
  inputs_.resetGameplayEvents(nowMs);
  if (!display_.prepareRound(selectedImage_)) {
    enterFault(display_.lastError());
    return;
  }
  startPrompt();
}

void Game::startPrompt() {
  expectedCommand_ = static_cast<Command>(esp_random() % 3);
  display_.showCommand(expectedCommand_, score_, responseWindowMs());
  if (!audio_.playCommand(expectedCommand_)) {
    enterFault(audio_.lastError());
    return;
  }
  state_ = GameState::Prompting;
}

void Game::handleCorrect(uint32_t nowMs) {
  ++score_;
  display_.revealNextTiles(Config::TILES_PER_SUCCESS);
  if (score_ >= Config::WINNING_SCORE) {
    display_.showVictory(score_);
    state_ = GameState::Victory;
    return;
  }

  display_.showSuccess(score_);
  feedbackUntilMs_ = nowMs + Config::SUCCESS_FEEDBACK_MS;
  state_ = GameState::SuccessFeedback;
}

void Game::finishRound(bool timedOut) {
  audio_.stop();
  display_.showGameOver(score_, timedOut);
  state_ = GameState::GameOver;
}

void Game::enterFault(const char *message) {
  audio_.stop();
  display_.showFault(message);
  state_ = GameState::HardwareFault;
}

uint32_t Game::responseWindowMs() const {
  const uint32_t reduction = score_ * Config::RESPONSE_DECREMENT_MS;
  if (reduction >=
      Config::INITIAL_RESPONSE_MS - Config::MINIMUM_RESPONSE_MS) {
    return Config::MINIMUM_RESPONSE_MS;
  }
  return Config::INITIAL_RESPONSE_MS - reduction;
}

