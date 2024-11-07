#ifndef _LIGHTS_H
#define _LIGHTS_H

#include "config.h"

extern uint8_t lightState[LIGHTS];

void lightsSetup();
void setLight(uint8_t light, uint16_t state);
uint8_t getLight(uint8_t light);
uint8_t toggleLight(uint8_t light);
void lightsLoop();

#endif
