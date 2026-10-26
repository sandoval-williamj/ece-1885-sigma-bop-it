#include "Audio.h"

#include <AudioFileSource.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>
#include <SdFat_Adafruit_Fork.h>

#include "Config.h"
#include "MediaCard.h"

namespace {

class SdFatAudioSource : public AudioFileSource {
 public:
  explicit SdFatAudioSource(const char *filename) { open(filename); }

  bool open(const char *filename) override {
    close();
    file_ = mediaCard.open(filename, O_RDONLY);
    return file_.isOpen();
  }

  uint32_t read(void *data, uint32_t length) override {
    const int bytesRead = file_.read(data, length);
    return bytesRead > 0 ? static_cast<uint32_t>(bytesRead) : 0;
  }

  bool seek(int32_t position, int direction) override {
    switch (direction) {
      case SEEK_SET:
        return position >= 0 && file_.seekSet(position);
      case SEEK_CUR:
        return file_.seekCur(position);
      case SEEK_END:
        return file_.seekEnd(position);
      default:
        return false;
    }
  }

  bool close() override {
    if (file_.isOpen()) {
      file_.close();
    }
    return true;
  }

  bool isOpen() override { return file_.isOpen(); }
  uint32_t getSize() override { return file_.fileSize(); }
  uint32_t getPos() override { return file_.curPosition(); }

 private:
  File32 file_;
};

}  // namespace

Audio::~Audio() {
  stop();
  delete output_;
}

bool Audio::begin() {
  if (!isMediaCardReady()) {
    lastError_ = "Audio cannot access the microSD card";
    return false;
  }

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
  return true;
}

const char *Audio::filenameFor(Command command) const {
  switch (command) {
    case Command::Snap:
      return Config::AUDIO_SNAP;
    case Command::Focus:
      return Config::AUDIO_FOCUS;
    case Command::Shake:
      return Config::AUDIO_SHAKE;
  }
  return nullptr;
}

bool Audio::playCommand(Command command) {
  stop();
  const char *filename = filenameFor(command);
  source_ = new SdFatAudioSource(filename);
  decoder_ = new AudioGeneratorWAV();
  if (source_ == nullptr || decoder_ == nullptr || !source_->isOpen()) {
    lastError_ = "Spoken command WAV is missing";
    Serial.print(F("ERROR opening audio file: "));
    Serial.println(filename);
    stop();
    return false;
  }

  if (!decoder_->begin(source_, output_)) {
    lastError_ = "Spoken command WAV could not be decoded";
    stop();
    return false;
  }
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

