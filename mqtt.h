#ifndef _MQTT_H
#define _MQTT_H

#define MQTT_QUEUE_SIZE 24

struct mqttQueueStruct {
  uint8_t type;
  uint8_t item;
  uint8_t state;
};

enum mqttPublishType {
  MQTT_PUBLISH_START = 1,
  MQTT_PUBLISH_SETTINGS = 2,
  MQTT_PUBLISH_LIGHT_STATE = 3,
  MQTT_PUBLISH_LIGHT_STATE_ALL = 4,
  MQTT_PUBLISH_BUTTON_STATE = 5
};

void mqttSetup();
void mqttLoop();

void mqttPublishStart();
void mqttPublishSettings(uint8_t buf1, uint8_t buf2);
void mqttPublishLightState(uint8_t light);
void mqttPublishLightStateAll();
void mqttPublishButtonState(uint8_t button, uint8_t state);

#endif
