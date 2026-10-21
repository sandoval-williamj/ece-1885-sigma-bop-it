#include "MediaCard.h"

#include <SPI.h>

#include "Config.h"

SdFat mediaCard;

namespace {
bool mediaCardReady = false;
}

bool beginMediaCard() {
  mediaCardReady =
      mediaCard.begin(Config::PIN_SD_CS, SD_SCK_MHZ(20));
  return mediaCardReady;
}

bool isMediaCardReady() { return mediaCardReady; }

