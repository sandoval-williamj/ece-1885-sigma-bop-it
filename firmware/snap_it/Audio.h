#pragma once

#include <Arduino.h>

#include "GameTypes.h"

class AudioFileSource;
class AudioGeneratorWAV;
class AudioOutputI2S;

class Audio {
 public:
  ~Audio();

  bool begin();
  bool playCommand(Command command);
  void update();
  void stop();

  bool isPlaying() const;
  bool isReady() const { return ready_; }
  const char *lastError() const { return lastError_; }

 private:
  const char *filenameFor(Command command) const;

  AudioFileSource *source_ = nullptr;
  AudioGeneratorWAV *decoder_ = nullptr;
  AudioOutputI2S *output_ = nullptr;
  bool ready_ = false;
  const char *lastError_ = "Audio has not been initialized";
};

