#pragma once

#include <Arduino.h>

namespace EmbeddedAssets {

struct ImageAsset {
  const char *label;
  const uint8_t *pixels;
  size_t byteCount;
};

struct AudioAsset {
  const uint8_t *data;
  size_t byteCount;
};

constexpr size_t IMAGE_COUNT = 3;
extern const ImageAsset IMAGES[IMAGE_COUNT];

extern const AudioAsset AUDIO_SNAP;
extern const AudioAsset AUDIO_FOCUS;
extern const AudioAsset AUDIO_SHAKE;

}  // namespace EmbeddedAssets
