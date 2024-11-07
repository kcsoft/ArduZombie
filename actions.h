#ifndef _ACTIONS_H
#define _ACTIONS_H

enum actions {
  ACTION_NONE = 0,
  ACTION_TOGGLE_LIGHT, // 1, param = light number
  ACTION_TOGGLE_BLINK, // 2, param = button number
  ACTION_MQTT_PUBLISH_1, // 3, param = button number
  ACTION_MQTT_PUBLISH_2, // 4, param = button number
  ACTION_MQTT_PUBLISH_3 // 5, param = button number
};

void executeAction(uint8_t action, uint8_t param);

#endif
