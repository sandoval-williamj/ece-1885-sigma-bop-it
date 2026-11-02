#include "EmbeddedAssets.h"

#include "Config.h"

#include "PITT01.inc"
#include "PITT02.inc"
#include "PITT03.inc"
#include "SNAP_WAV.inc"
#include "FOCUS_WAV.inc"
#include "SHAKE_WAV.inc"

namespace {

constexpr size_t EXPECTED_IMAGE_BYTES =
    Config::IMAGE_WIDTH * Config::IMAGE_HEIGHT * sizeof(uint16_t);

static_assert(PITT01_RGB565_SIZE == EXPECTED_IMAGE_BYTES,
              "PITT01 has the wrong dimensions");
static_assert(PITT02_RGB565_SIZE == EXPECTED_IMAGE_BYTES,
              "PITT02 has the wrong dimensions");
static_assert(PITT03_RGB565_SIZE == EXPECTED_IMAGE_BYTES,
              "PITT03 has the wrong dimensions");

}  // namespace

namespace EmbeddedAssets {

const ImageAsset IMAGES[IMAGE_COUNT] = {
    {"PITT CAMPUS", PITT01_RGB565, PITT01_RGB565_SIZE},
    {"HEINZ CHAPEL", PITT02_RGB565, PITT02_RGB565_SIZE},
    {"PITT PANTHER", PITT03_RGB565, PITT03_RGB565_SIZE},
};

const AudioAsset AUDIO_SNAP = {SNAP_WAV, SNAP_WAV_SIZE};
const AudioAsset AUDIO_FOCUS = {FOCUS_WAV, FOCUS_WAV_SIZE};
const AudioAsset AUDIO_SHAKE = {SHAKE_WAV, SHAKE_WAV_SIZE};

}  // namespace EmbeddedAssets
