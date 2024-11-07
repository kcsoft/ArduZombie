#include <Arduino.h>

#include "actions.h"
#include "mqtt.h"
#include "lights.h"
#include "blink.h"

// called from ISR (on button presses)
void executeAction(uint8_t action, uint8_t param) {
  switch (action) {
    case ACTION_NONE:
      break;
    case ACTION_TOGGLE_LIGHT:
      toggleLight(param);
      break;
    case ACTION_TOGGLE_BLINK:
      toggleBlinkEnabled(param);
      break;
    case ACTION_MQTT_PUBLISH_1:
    case ACTION_MQTT_PUBLISH_2:
    case ACTION_MQTT_PUBLISH_3:
      mqttPublishButtonState(param, action - ACTION_MQTT_PUBLISH_1 + 1);
      break;
    default:
      break;
  }
}
