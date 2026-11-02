#include "Audio.h"

#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>

#include "Config.h"
#include "EmbeddedAssets.h"

namespace {

const EmbeddedAssets::AudioAsset &assetForCommand(Command command) {
  switch (command) {
    case Command::Snap:
      return EmbeddedAssets::AUDIO_SNAP;
    case Command::Focus:
      return EmbeddedAssets::AUDIO_FOCUS;
    case Command::Shake:
      return EmbeddedAssets::AUDIO_SHAKE;
  }
  return EmbeddedAssets::AUDIO_SNAP;
}

}  // namespace

Audio::~Audio() {
  stop();
  delete output_;
}

bool Audio::begin() {
  output_ = new AudioOutputI2S();
  if (output_ == nullptr ||
      !output_->SetPinout(Config::PIN_I2S_BCLK, Config::PIN_I2S_LRCLK,
                          Config::PIN_I2S_DATA)) {
    lastError_ = "I2S output initialization failed";
    delete output_;
    output_ = nullptr;
    return false;
  }
  output_->SetOutputModeMono(true);
  output_->SetGain(Config::AUDIO_GAIN);
  ready_ = true;
  lastError_ = "";
  Serial.println(F("Audio ready; voice prompts are embedded in flash."));
  return true;
}

bool Audio::playCommand(Command command) {
  stop();
  const EmbeddedAssets::AudioAsset &asset = assetForCommand(command);
  source_ = new AudioFileSourcePROGMEM(asset.data, asset.byteCount);
  decoder_ = new AudioGeneratorWAV();
  if (source_ == nullptr || decoder_ == nullptr || !source_->isOpen()) {
    lastError_ = "Embedded spoken command is unavailable";
    stop();
    return false;
  }

  if (!decoder_->begin(source_, output_)) {
    lastError_ = "Embedded spoken command could not be decoded";
    stop();
    return false;
  }

  Serial.print(F("Playing embedded voice prompt: "));
  Serial.print(commandLabel(command));
  Serial.print(F(" ("));
  Serial.print(asset.byteCount);
  Serial.println(F(" bytes)"));
  return true;
}

void Audio::update() {
  if (decoder_ == nullptr) {
    return;
  }
  if (!decoder_->isRunning() || !decoder_->loop()) {
    stop();
  }
}

bool Audio::isPlaying() const {
  return decoder_ != nullptr && decoder_->isRunning();
}

void Audio::stop() {
  if (decoder_ != nullptr) {
    if (decoder_->isRunning()) {
      decoder_->stop();
    }
    delete decoder_;
    decoder_ = nullptr;
  }
  if (source_ != nullptr) {
    source_->close();
    delete source_;
    source_ = nullptr;
  }
}
