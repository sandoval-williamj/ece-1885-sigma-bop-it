#pragma once

#include <Arduino.h>
#include <SdFat_Adafruit_Fork.h>

extern SdFat mediaCard;

bool beginMediaCard();
bool isMediaCardReady();

