#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include <Arduino.h>
#include <ESP32Encoder.h>
#include <SPI.h>
#include <esp_system.h>

#include "PITT01.inc"
#include "PITT02.inc"
#include "PITT03.inc"

namespace {

// Working ESP32-S3 DevKitC-1 bench pinout. The display uses write-only SPI;
// neither its MISO pin nor a microSD card is needed by this sketch.
constexpr uint8_t PIN_SNAP_BUTTON = 5;
constexpr uint8_t PIN_SNAP_LED = 6;
constexpr uint8_t PIN_SD_CS = 8;
constexpr uint8_t PIN_TFT_DC = 9;
constexpr uint8_t PIN_TFT_CS = 10;
constexpr uint8_t PIN_SPI_MOSI = 11;
constexpr uint8_t PIN_SPI_SCK = 12;
constexpr uint8_t PIN_TFT_RESET = 14;
constexpr uint8_t PIN_ENCODER_A = 15;
constexpr uint8_t PIN_ENCODER_B = 16;
constexpr uint8_t PIN_BUZZER = 17;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t TFT_SPI_HZ = 8000000;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 25;
constexpr int64_t ENCODER_COUNTS_PER_ACTION = 2;
constexpr uint32_t LED_PULSE_MS = 150;
constexpr uint8_t PWM_RESOLUTION_BITS = 10;

constexpr uint32_t BOOT_SCREEN_MS = 1400;
constexpr uint32_t INITIAL_RESPONSE_MS = 5000;
constexpr uint32_t MINIMUM_RESPONSE_MS = 1000;
constexpr uint32_t RESPONSE_DECREMENT_MS = 40;
constexpr uint32_t SUCCESS_FEEDBACK_MS = 350;
constexpr uint8_t WINNING_SCORE = 99;

constexpr int16_t SCREEN_WIDTH = 480;
constexpr int16_t STATUS_HEIGHT = 32;
constexpr int16_t IMAGE_WIDTH = 480;
constexpr int16_t IMAGE_HEIGHT = 288;
constexpr uint32_t PIXEL_COUNT =
    static_cast<uint32_t>(IMAGE_WIDTH) * IMAGE_HEIGHT;
constexpr uint16_t PIXELS_PER_CORRECT =
    (PIXEL_COUNT + WINNING_SCORE - 1) / WINNING_SCORE;
constexpr size_t IMAGE_BYTES = PIXEL_COUNT * sizeof(uint16_t);

static_assert(PITT01_RGB565_SIZE == IMAGE_BYTES, "PITT01 has the wrong size");
static_assert(PITT02_RGB565_SIZE == IMAGE_BYTES, "PITT02 has the wrong size");
static_assert(PITT03_RGB565_SIZE == IMAGE_BYTES, "PITT03 has the wrong size");

enum class Command : uint8_t { Snap, Focus };
enum class GameState : uint8_t {
  Boot,
  AwaitingInput,
  SuccessFeedback,
  GameOver,
  Victory,
};

struct GameImage {
  const char *label;
  const uint8_t *pixels;
};

constexpr GameImage GAME_IMAGES[] = {
    {"PITT CAMPUS", PITT01_RGB565},
    {"HEINZ CHAPEL", PITT02_RGB565},
    {"PITT PANTHER", PITT03_RGB565},
};
constexpr size_t IMAGE_COUNT = sizeof(GAME_IMAGES) / sizeof(GAME_IMAGES[0]);

Adafruit_HX8357 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RESET);
ESP32Encoder encoder;

GameState gameState = GameState::Boot;
Command expectedCommand = Command::Snap;
size_t imageIndex = IMAGE_COUNT;
uint8_t score = 0;

uint32_t stateDeadlineMs = 0;
uint32_t responseStartedMs = 0;

bool lastRawButtonState = HIGH;
bool stableButtonState = HIGH;
uint32_t lastRawButtonChangeAtMs = 0;
bool snapPressPending = false;
bool focusActionPending = false;
int64_t encoderBaseline = 0;

bool ledIsOn = false;
uint32_t ledOffAtMs = 0;
bool buzzerReady = false;
bool toneIsPlaying = false;
uint32_t toneOffAtMs = 0;

// These three values define a permutation of every image pixel. Moving by a
// step that is coprime with PIXEL_COUNT visits every pixel exactly once.
uint32_t revealedPixelCount = 0;
uint32_t nextPixelIndex = 0;
uint32_t pixelStep = 1;

const char *commandLabel(Command command) {
  return command == Command::Snap ? "SNAP" : "FOCUS";
}

uint32_t greatestCommonDivisor(uint32_t a, uint32_t b) {
  while (b != 0) {
    const uint32_t remainder = a % b;
    a = b;
    b = remainder;
  }
  return a;
}

uint32_t responseWindowMs() {
  const uint32_t reduction = score * RESPONSE_DECREMENT_MS;
  const uint32_t maximumReduction = INITIAL_RESPONSE_MS - MINIMUM_RESPONSE_MS;
  return reduction >= maximumReduction ? MINIMUM_RESPONSE_MS
                                        : INITIAL_RESPONSE_MS - reduction;
}

void drawStatus(const char *message, uint16_t background) {
  tft.fillRect(0, 0, SCREEN_WIDTH, STATUS_HEIGHT, background);
  tft.setTextWrap(false);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(2);
  tft.setCursor(8, 8);
  tft.print(message);
  tft.setCursor(402, 8);
  tft.print(F("S:"));
  tft.print(score);
}

void drawBootScreen() {
  tft.fillScreen(HX8357_BLACK);
  tft.setTextWrap(false);
  tft.setTextColor(HX8357_CYAN);
  tft.setTextSize(4);
  tft.setCursor(64, 88);
  tft.print(F("2-INPUT SNAP IT"));

  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(2);
  tft.setCursor(104, 158);
  tft.print(F("SNAP button + FOCUS dial"));
  tft.setCursor(142, 194);
  tft.print(F("Starting automatically"));
}

void startLedPulse(uint32_t nowMs) {
  digitalWrite(PIN_SNAP_LED, HIGH);
  ledIsOn = true;
  ledOffAtMs = nowMs + LED_PULSE_MS;
}

void updateLed(uint32_t nowMs) {
  if (ledIsOn && static_cast<int32_t>(nowMs - ledOffAtMs) >= 0) {
    digitalWrite(PIN_SNAP_LED, LOW);
    ledIsOn = false;
  }
}

void playTone(uint16_t frequencyHz, uint32_t durationMs, uint32_t nowMs) {
  if (!buzzerReady) {
    return;
  }
  ledcWriteTone(PIN_BUZZER, frequencyHz);
  toneIsPlaying = true;
  toneOffAtMs = nowMs + durationMs;
}

void stopTone() {
  if (buzzerReady) {
    ledcWriteTone(PIN_BUZZER, 0);
  }
  toneIsPlaying = false;
}

void updateTone(uint32_t nowMs) {
  if (toneIsPlaying && static_cast<int32_t>(nowMs - toneOffAtMs) >= 0) {
    stopTone();
  }
}

void updateButton(uint32_t nowMs) {
  const bool rawButtonState = digitalRead(PIN_SNAP_BUTTON);
  if (rawButtonState != lastRawButtonState) {
    lastRawButtonState = rawButtonState;
    lastRawButtonChangeAtMs = nowMs;
  }

  if (rawButtonState == stableButtonState ||
      nowMs - lastRawButtonChangeAtMs < BUTTON_DEBOUNCE_MS) {
    return;
  }

  stableButtonState = rawButtonState;
  if (stableButtonState == LOW) {
    snapPressPending = true;
    startLedPulse(nowMs);
  }
}

void updateEncoder() {
  const int64_t currentCount = encoder.getCount();
  const int64_t delta = currentCount - encoderBaseline;
  if (delta >= ENCODER_COUNTS_PER_ACTION ||
      delta <= -ENCODER_COUNTS_PER_ACTION) {
    focusActionPending = true;
    encoderBaseline = currentCount;
  }
}

void clearInputEvents() {
  snapPressPending = false;
  focusActionPending = false;
  encoderBaseline = encoder.getCount();
}

bool takeSnapPress() {
  if (!snapPressPending) {
    return false;
  }
  snapPressPending = false;
  return true;
}

bool takeGameplayAction(Command &action) {
  if (snapPressPending) {
    snapPressPending = false;
    action = Command::Snap;
    return true;
  }
  if (focusActionPending) {
    focusActionPending = false;
    action = Command::Focus;
    return true;
  }
  return false;
}

void randomizePixelOrder() {
  revealedPixelCount = 0;
  nextPixelIndex = esp_random() % PIXEL_COUNT;
  do {
    pixelStep = 1 + (esp_random() % (PIXEL_COUNT - 1));
  } while (greatestCommonDivisor(pixelStep, PIXEL_COUNT) != 1);

  Serial.print(F("Pixel permutation start="));
  Serial.print(nextPixelIndex);
  Serial.print(F(" step="));
  Serial.println(pixelStep);
}

void revealRandomPixels() {
  const GameImage &image = GAME_IMAGES[imageIndex];
  const uint16_t *pixels =
      reinterpret_cast<const uint16_t *>(image.pixels);
  const uint32_t remaining = PIXEL_COUNT - revealedPixelCount;
  const uint32_t pixelsToDraw =
      remaining < PIXELS_PER_CORRECT ? remaining : PIXELS_PER_CORRECT;

  tft.startWrite();
  for (uint32_t i = 0; i < pixelsToDraw; ++i) {
    const int16_t x = nextPixelIndex % IMAGE_WIDTH;
    const int16_t y = STATUS_HEIGHT + (nextPixelIndex / IMAGE_WIDTH);
    const uint16_t color = pgm_read_word(pixels + nextPixelIndex);
    tft.writePixel(x, y, color);
    nextPixelIndex += pixelStep;
    if (nextPixelIndex >= PIXEL_COUNT) {
      nextPixelIndex %= PIXEL_COUNT;
    }
  }
  tft.endWrite();
  revealedPixelCount += pixelsToDraw;
}

void issuePrompt(uint32_t nowMs) {
  clearInputEvents();
  expectedCommand =
      static_cast<Command>(esp_random() % 2);
  const char *message =
      expectedCommand == Command::Snap ? "SNAP IT!" : "FOCUS IT!";
  drawStatus(message, expectedCommand == Command::Snap ? HX8357_BLUE
                                                       : HX8357_MAGENTA);
  playTone(expectedCommand == Command::Snap ? 2200 : 1400, 90, nowMs);
  responseStartedMs = nowMs;
  gameState = GameState::AwaitingInput;

  Serial.print(F("Command: "));
  Serial.print(commandLabel(expectedCommand));
  Serial.print(F(" | score="));
  Serial.print(score);
  Serial.print(F(" | window="));
  Serial.print(responseWindowMs());
  Serial.println(F(" ms"));
}

void chooseRandomImage() {
  if (imageIndex >= IMAGE_COUNT) {
    imageIndex = esp_random() % IMAGE_COUNT;
  } else {
    // Still random, but avoid showing the same photograph in two consecutive
    // rounds so every restart visibly tests a new source image.
    imageIndex = (imageIndex + 1 + (esp_random() % (IMAGE_COUNT - 1))) %
                 IMAGE_COUNT;
  }
}

void startRound(uint32_t nowMs) {
  stopTone();
  score = 0;
  chooseRandomImage();
  randomizePixelOrder();
  clearInputEvents();
  tft.fillRect(0, STATUS_HEIGHT, IMAGE_WIDTH, IMAGE_HEIGHT, HX8357_BLACK);

  Serial.println();
  Serial.print(F("New round image: "));
  Serial.println(GAME_IMAGES[imageIndex].label);
  issuePrompt(nowMs);
}

void handleCorrect(uint32_t nowMs) {
  ++score;
  revealRandomPixels();
  Serial.print(F("Correct. Revealed pixels: "));
  Serial.print(revealedPixelCount);
  Serial.print('/');
  Serial.println(PIXEL_COUNT);

  if (revealedPixelCount >= PIXEL_COUNT || score >= WINNING_SCORE) {
    drawStatus("WIN! SNAP=RESTART", HX8357_GREEN);
    playTone(3200, 500, nowMs);
    gameState = GameState::Victory;
    Serial.println(F("Victory: complete image revealed."));
    return;
  }

  drawStatus("NICE!", HX8357_GREEN);
  playTone(2800, 80, nowMs);
  stateDeadlineMs = nowMs + SUCCESS_FEEDBACK_MS;
  gameState = GameState::SuccessFeedback;
}

void finishRound(bool timedOut, Command received, uint32_t nowMs) {
  stopTone();
  drawStatus(timedOut ? "TIMEOUT - SNAP RESTART"
                      : "WRONG - SNAP RESTART",
             HX8357_RED);
  playTone(500, 350, nowMs);
  gameState = GameState::GameOver;

  if (timedOut) {
    Serial.print(F("Game over: timeout waiting for "));
    Serial.println(commandLabel(expectedCommand));
  } else {
    Serial.print(F("Game over: expected "));
    Serial.print(commandLabel(expectedCommand));
    Serial.print(F(", received "));
    Serial.println(commandLabel(received));
  }
  Serial.println(F("Press SNAP to start a new round."));
}

}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(250);
  Serial.println();
  Serial.println(F("Two-input SNAP IT game test"));
  Serial.println(F("Inputs: SNAP button and FOCUS encoder"));
  Serial.println(F("Images are embedded in flash; microSD is not required."));

  pinMode(PIN_SNAP_BUTTON, INPUT_PULLUP);
  pinMode(PIN_SNAP_LED, OUTPUT);
  digitalWrite(PIN_SNAP_LED, LOW);
  lastRawButtonState = digitalRead(PIN_SNAP_BUTTON);
  stableButtonState = lastRawButtonState;

  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  encoder.attachHalfQuad(PIN_ENCODER_A, PIN_ENCODER_B);
  encoder.setCount(0);
  encoderBaseline = encoder.getCount();

  buzzerReady = ledcAttach(PIN_BUZZER, 2000, PWM_RESOLUTION_BITS);
  if (!buzzerReady) {
    Serial.println(F("Buzzer PWM unavailable; continuing without sound."));
  }

  pinMode(PIN_TFT_CS, OUTPUT);
  digitalWrite(PIN_TFT_CS, HIGH);
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);
  SPI.begin(PIN_SPI_SCK, -1, PIN_SPI_MOSI);
  tft.begin(TFT_SPI_HZ);
  tft.setRotation(1);
  drawBootScreen();

  stateDeadlineMs = millis() + BOOT_SCREEN_MS;
}

void loop() {
  const uint32_t nowMs = millis();
  updateButton(nowMs);
  updateEncoder();
  updateLed(nowMs);
  updateTone(nowMs);

  switch (gameState) {
    case GameState::Boot:
      if (static_cast<int32_t>(nowMs - stateDeadlineMs) >= 0) {
        startRound(nowMs);
      }
      break;

    case GameState::AwaitingInput: {
      Command received = Command::Snap;
      if (takeGameplayAction(received)) {
        if (received == expectedCommand) {
          handleCorrect(nowMs);
        } else {
          finishRound(false, received, nowMs);
        }
      } else if (nowMs - responseStartedMs >= responseWindowMs()) {
        finishRound(true, received, nowMs);
      }
      break;
    }

    case GameState::SuccessFeedback:
      if (static_cast<int32_t>(nowMs - stateDeadlineMs) >= 0) {
        issuePrompt(nowMs);
      }
      break;

    case GameState::GameOver:
    case GameState::Victory:
      if (takeSnapPress()) {
        startRound(nowMs);
      }
      break;
  }
}
