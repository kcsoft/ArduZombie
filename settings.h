#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "config.h"

struct Settings {
  uint16_t crc;
  char id;
  char hostname[20];
  unsigned char blinkEnabled[BUTTONS / 8];
  // actions are 8 bit, 4H = actionType, 4L = param (light no or button no)
  unsigned char buttonShortActions[BUTTONS];
  unsigned char buttonMediumActions[BUTTONS];
  unsigned char buttonLongActions[BUTTONS];
};

extern Settings settings;

void setDefaultSettings();
void loadSettings();
void saveSettings();
unsigned char *getSettingsString(unsigned char *settingField);

#endif
