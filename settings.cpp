#include <Arduino.h>
#include <EEPROM.h>
#include <util/crc16.h>
#include <string.h>

#include "config.h"
#include "settings.h"
#include "actions.h"

Settings settings;

// buffer for settings string
unsigned char stringSettings[128];

uint16_t calcCRC(uint8_t* buf, uint8_t len) {
  uint16_t crc = 0;
  while (--len)
    crc = _crc16_update(crc, *buf++);
  return crc;
}

void setDefaultSettings() {
  uint8_t i;

  settings.id = DEFAULT_ID;
  strncpy(settings.hostname, DEFAULT_HOSTNAME, 20);

  i = 0;
  while (i < BUTTONS / 8) { // set all buttons blinking
    settings.blinkEnabled[i] = 255;
    i++;
  }

  i = 0;
  while (i < BUTTONS) { // set all buttons to default actions
    settings.buttonShortActions[i] = ACTION_TOGGLE_LIGHT << 4 | i;
    settings.buttonMediumActions[i] = ACTION_TOGGLE_BLINK << 4 | i;
    settings.buttonLongActions[i] = ACTION_MQTT_PUBLISH_3 << 4 | i;
    i++;
  }
}

void loadSettings() {
  EEPROM.get(EEPROM_START, settings);

  // invalid settings, write defaults
  if (calcCRC((uint8_t *)&settings + 2, sizeof(Settings) - 2) != settings.crc) {
    DEBUG_PRINTLN("Invalid settings");
    setDefaultSettings();
    settings.crc = calcCRC((uint8_t *)&settings + 2, sizeof(Settings) - 2);
    EEPROM.put(EEPROM_START, settings);
  }
}

void saveSettings() {
  settings.crc = calcCRC((uint8_t *)&settings + 2, sizeof(Settings) - 2);
  EEPROM.put(EEPROM_START, settings);
}

/**
 * param settingFields - pointer to the requested setting:
 *  i - id
 *  h - hostname
 *  b - blinkEnabled
 *  as - buttonShortActions
 *  am - buttonMediumActions
 *  al - buttonLongActions
 *
 * put in stringSettings the settings in a string format:
 * i:id hHostname bBlinkEnabled asShortActions amMediumActions alLongActions
*/
unsigned char *getSettingsString(unsigned char *settingField) {
  uint8_t i, j, b, len, mask;

  i = 0;
  stringSettings[i] = 0;

  if (settingField[0] == 'i') { // id
    stringSettings[i++] = 'i';
    stringSettings[i++] = settings.id;
    stringSettings[i] = 0;
    return stringSettings;
  }

  if (settingField[0] == 'h') {
    stringSettings[i++] = 'h';
    len = strlen(settings.hostname);
    strncpy((char *)stringSettings + i, settings.hostname, len);
    i += len;
    stringSettings[i] = 0;
    return stringSettings;
  }

  if (settingField[0] == 'b') {
    stringSettings[i++] = 'b';
    // send the next BUTTONS bytes as binary string
    j = BUTTONS / 8;
    while (j) {
      j--;
      mask = 128;
      while (mask) {
        stringSettings[i++] = settings.blinkEnabled[j] & mask ? '1' : '0';
        mask /= 2;
      }
    }
    stringSettings[i] = 0;
    return stringSettings;
  }

  if (settingField[0] == 'a' && settingField[1] == 's') {
    stringSettings[i++] = 'a';
    stringSettings[i++] = 's';
    for (b = 0; b < BUTTONS; b++) {
      stringSettings[i++] = ' ';
      stringSettings[i++] = b > 9 ? b + 55 : b + 48;
      j = settings.buttonShortActions[b] >> 4;
      stringSettings[i++] = j > 9 ? j + 55 : j + 48;
      j = settings.buttonShortActions[b] & 0x0F;
      stringSettings[i++] = j > 9 ? j + 55 : j + 48;
    }
    stringSettings[i] = 0;
    return stringSettings;
  }

  if (settingField[0] == 'a' && settingField[1] == 'm') {
    stringSettings[i++] = 'a';
    stringSettings[i++] = 'm';
    for (b = 0; b < BUTTONS; b++) {
      stringSettings[i++] = ' ';
      stringSettings[i++] = b > 9 ? b + 55 : b + 48;
      j = settings.buttonMediumActions[b] >> 4;
      stringSettings[i++] = j > 9 ? j + 55 : j + 48;
      j = settings.buttonMediumActions[b] & 0x0F;
      stringSettings[i++] = j > 9 ? j + 55 : j + 48;
    }
    stringSettings[i] = 0;
    return stringSettings;
  }

  if (settingField[0] == 'a' && settingField[1] == 'l') {
    stringSettings[i++] = 'a';
    stringSettings[i++] = 'l';
    for (b = 0; b < BUTTONS; b++) {
      stringSettings[i++] = ' ';
      stringSettings[i++] = b > 9 ? b + 55 : b + 48;
      j = settings.buttonLongActions[b] >> 4;
      stringSettings[i++] = j > 9 ? j + 55 : j + 48;
      j = settings.buttonLongActions[b] & 0x0F;
      stringSettings[i++] = j > 9 ? j + 55 : j + 48;
    }
    stringSettings[i] = 0;
    return stringSettings;
  }

  return stringSettings;
}
