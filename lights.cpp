#include <Arduino.h>

#include "lights.h"
#include "mqtt.h"
#include "settings.h"

const uint8_t lightIOPins[LIGHTS] = {
  A8, A9, A10, A11, A12, A13, A14, A15,
  42, 43, 44, 45, 46, 47, 48, 49
};

uint8_t lightState[LIGHTS];
uint16_t lightOnCounter[LIGHTS];

void setLight(uint8_t light, uint16_t  state) {
  lightState[light] = (state == 0 ? LOW : HIGH);
  lightOnCounter[light] = (state > 1 ? state : 0);
  digitalWrite(lightIOPins[light], lightState[light]);
  mqttPublishLightState(light);
}

uint8_t getLight(uint8_t light) {
  return lightState[light];
}

uint8_t toggleLight(uint8_t light) {
  setLight(light, lightState[light] == LOW ? 1 : 0);
  return lightState[light];
}

void lightsSetup() {
  uint8_t light;

  light = 0;
  while (light < LIGHTS) { // set lights as output and turn off
    pinMode(lightIOPins[light], OUTPUT);
    digitalWrite(lightIOPins[light], LOW);
    lightState[light] = LOW;
    lightOnCounter[light] = 0;
    light++;
  }
}

// call this every second
void lightsLoop() {
  uint8_t light;
  light = 0;
  while (light < LIGHTS) {
    if (lightOnCounter[light] > 0) {
      if (--lightOnCounter[light] == 0) { // turn off light
        setLight(light, 0);
      }
    }
    light++;
  }
}
