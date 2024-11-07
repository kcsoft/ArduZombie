#include <avr/wdt.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#define CIRCULAR_BUFFER_INT_SAFE
#include <CircularBuffer.hpp>

#include "config.h"
#include "settings.h"
#include "dhcp.h"
#include "mqtt.h"
#include "lights.h"
#include "blink.h"
#include "status.h"

// subscribe
char mqttLightTopic[] = "house/\0/light/\0\0\0";
char mqttBlinkTopic[] = "house/\0/blink/\0\0\0";
char mqttStatusTopicAsk[] = "house/\0\0";
char mqttSettingTopic[] = "house/\0/set\0";
char mqttResetTopic[] = "house/\0/reset\0";
// publish
char mqttStartTopic[] = "house/\0/start\0";
char mqttLightStateTopic[] = "house/\0/light/state/\0\0\0";
char mqttButtonTopic[] = "house/\0/button/\0\0\0";
char mqttSendSettingTopic[] = "house/\0/settings\0";

CircularBuffer<mqttQueueStruct, MQTT_QUEUE_SIZE> mqttQueue;

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
uint16_t mqttStateTopicVal;

void mqttPublishStart() {
  mqttQueue.push({MQTT_PUBLISH_START, 0, 0});
}

void mqttPublishSettings(uint8_t buf1, uint8_t buf2) {
  mqttQueue.push({MQTT_PUBLISH_SETTINGS, buf1, buf2});
}

void mqttPublishLightState(uint8_t light) {
  mqttQueue.push({MQTT_PUBLISH_LIGHT_STATE, light, 0});
}

// send all ON lights
void mqttPublishLightStateAll() {
  mqttQueue.push({MQTT_PUBLISH_LIGHT_STATE_ALL, 0, 0});
}

void mqttPublishButtonState(uint8_t button, uint8_t state) {
  mqttQueue.push({MQTT_PUBLISH_BUTTON_STATE, button, state});
}

uint8_t setupTopicForItem(char *topic, uint8_t idPosition, uint8_t item) {
  uint8_t len = idPosition > 0 ? idPosition : strlen(topic);
  if (item >= 10) {
    topic[len] = '1';
    topic[len + 1] = 0x30 + item - 10;
    topic[len + 2] = 0;
  } else {
    topic[len] = 0x30 + item;
    topic[len + 1] = 0;
  }
  return len;
}

// helper function to convert string to int
const unsigned int fast_atoi_map [] = {
  0, 10, 20, 30, 40, 50, 60, 70, 80, 90,
  0, 100, 200, 300, 400, 500, 600, 700, 800, 900,
  0, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000,
  0, 10000, 20000, 30000, 40000, 50000, 60000
};
unsigned int fast_atoi(char *str, unsigned char length) {
  unsigned int val = 0;
  const unsigned int *imap = fast_atoi_map + (length - 2) * 10;
  while (length-- > 1) {
    val = val + *(imap + (*str++ - '0'));
    imap -= 10;
  }
  return val + (*str - '0');
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  uint8_t myTopicLen, channel;
  uint8_t button, action, param;
  uint8_t topicLen = strlen(topic);

  DEBUG_PRINT(topic);
  DEBUG_PRINT(":");
  DEBUG_PRINTLN((char *)payload);

  // mqttLightTopic
  myTopicLen = strlen(mqttLightTopic);
  if (topicLen > myTopicLen && strncmp(mqttLightTopic, topic, myTopicLen) == 0) {
    if (topicLen - myTopicLen == 1) {
      channel = topic[myTopicLen] - '0';
    } else {
      channel = (topic[myTopicLen] - '0') * 10 + (topic[myTopicLen + 1] - '0');
    }
    if (channel > LIGHTS)
      return;

    if (channel == 0) { // turn all on/off
      // topic value
      myTopicLen = (length == 1 && payload[0] == '1') ? HIGH : LOW;
      if (myTopicLen == LOW) { // only turn off
        while (channel < LIGHTS) {
          setLight(channel, myTopicLen); // TODO: check relay no for all setLight calls
          channel++;
        }
      }
    } else { // turn specific channel on/off
      channel--;
      if (length == 1 && (payload[0] == '1' || payload[0] == '0')) {
        setLight(channel, (payload[0] == '1') ? HIGH : LOW);
      } else { // message is a counter > 1
        setLight(channel, fast_atoi((char *)payload, length));
      }
    }

    return;
  }

  // mqttStatusTopicAsk - send all ON lights
  if (strcmp(mqttStatusTopicAsk, topic) == 0) {
    mqttPublishLightStateAll();
    return;
  }

  // mqttSettingTopic:
  // iX - set id
  // hHOSTNAME - set hostname
  // asXYZ - set action short for button X (0-F), action Y (0-5), param Z (0-F)
  // amXYZ - set action medium for button X, action Y, param Z
  // alXYZ - set action long for button X, action Y, param Z
  // ? - reset to default settings
  // b0 .. b4 - set blink mode [no eeprom save]
  if (strcmp(mqttSettingTopic, topic) == 0) {
    if (payload[0] == 'b') { // set blink mode
      setBlinkMode(payload[1] - '0');
      return;
    }
    if (payload[0] == '?') { // send settings string
      mqttPublishSettings(payload[1], payload[2]);
      return;
    }

    if (payload[0] == 'i') { // set id
      settings.id = payload[1];
    } else
    if (payload[0] == 'h') { // set hostname
      length = length >= 20 ? 20 : length;
      strncpy(settings.hostname, (char *)payload + 1, length - 1);
      settings.hostname[length - 1] = 0;
    } else
    if (payload[0] == 'a' // set button actions
      && (payload[1] == 's' || payload[1] == 'm' || payload[1] == 'l') // short, medium or long
      && (payload[2] >= '0' && payload[2] <= 'F') // button number
      && (payload[3] >= '0' && payload[3] <= 'F') // action number
      && (payload[4] >= '0' && payload[4] <= 'F') // param number
    ) {
      button = (payload[2] > '9' ? (payload[2] - 'A' + 10) : (payload[2] - '0')) & 0x0F;
      action = (payload[3] > '9' ? (payload[3] - 'A' + 10) : (payload[3] - '0')) & 0x0F;
      param = (payload[4] > '9' ? (payload[4] - 'A' + 10) : (payload[4] - '0')) & 0x0F;
      if (payload[1] == 's') {
        settings.buttonShortActions[button] = (action << 4) | param;
      } else
      if (payload[1] == 'm') {
        settings.buttonMediumActions[button] = (action << 4) | param;
      } else
      if (payload[1] == 'l') {
        settings.buttonLongActions[button] = (action << 4) | param;
      }
    } else
    if (payload[0] == '0') { // reset to default
      setDefaultSettings();
    } else { // no settings changed
      return;
    }

    saveSettings();
  }

  // reset using watchdog
  if (strcmp(mqttResetTopic, topic) == 0) {
    noInterrupts();
    wdt_enable(WDTO_15MS);
    while (1) {}
  }
}

void mqttSetup() {
  // prepare topics, replace ID
  uint8_t idPosition = strlen(mqttLightTopic);
  mqttLightTopic[idPosition] = settings.id;
  mqttBlinkTopic[idPosition] = settings.id;
  mqttStatusTopicAsk[idPosition] = settings.id;
  mqttSettingTopic[idPosition] = settings.id;
  mqttLightStateTopic[idPosition] = settings.id;
  mqttResetTopic[idPosition] = settings.id;
  mqttButtonTopic[idPosition] = settings.id;
  mqttStartTopic[idPosition] = settings.id;
  mqttSendSettingTopic[idPosition] = settings.id;

  mqttClient.setServer(MQTT_HOST, 1883);
  mqttClient.setCallback(mqttCallback);
}

void mqttLoop() {
  if (!mqttClient.connected()) {
    if (mqttClient.connect(settings.id)) {
      DEBUG_PRINTLN("MQTT cnected");
      setStatus(STATUS_MQTT_CONNECTED);
      uint8_t idPosition = 0, i = 0;
      while (i <= LIGHTS) {
        idPosition = setupTopicForItem(mqttLightTopic, idPosition, i);
        mqttClient.subscribe((char *)mqttLightTopic);
        mqttLightTopic[idPosition] = 0;
        i++;
      }

      // subscribe to ask lights states
      mqttClient.subscribe((char *)mqttStatusTopicAsk);

      // add settings topic. /house/1/set i1, /house/1/set hMyHostName
      mqttClient.subscribe((char *)mqttSettingTopic);

      // add reset topic /house/1/reset
      mqttClient.subscribe((char *)mqttResetTopic);

      mqttClient.publish(mqttStartTopic, "1");
    } else {
      if (getStatus() == STATUS_MQTT_CONNECTED) {
        setStatus(STATUS_DHCP_CONNECTED);
      }
    }
  }

  // publish messages
  uint8_t idPosition;
  uint8_t maxPublish = 5;
  uint8_t tmpBuf[2];
  while (maxPublish-- > 0 && !mqttQueue.isEmpty()) {
    mqttQueueStruct data = mqttQueue.pop();
    switch (data.type) {
      case MQTT_PUBLISH_START:
        mqttClient.publish(mqttStartTopic, "1");
        break;
      case MQTT_PUBLISH_SETTINGS:
        tmpBuf[0] = data.item;
        tmpBuf[1] = data.state;
        mqttClient.publish(mqttSendSettingTopic, getSettingsString(tmpBuf));
        break;
      case MQTT_PUBLISH_LIGHT_STATE:
        idPosition = setupTopicForItem(mqttLightStateTopic, 0, data.item + 1);
        mqttStateTopicVal = getLight(data.item) ? '1' : '0'; // uint16 has 0 second byte
        mqttClient.publish(mqttLightStateTopic, (char *)(&mqttStateTopicVal));
        mqttLightStateTopic[idPosition] = 0;
        break;
      case MQTT_PUBLISH_LIGHT_STATE_ALL: // publish ON lights
        for (uint8_t i = 0; i < LIGHTS; i++) {
          if (getLight(i) == HIGH) {
            idPosition = setupTopicForItem(mqttLightStateTopic, 0, i + 1);
            mqttStateTopicVal = '1';
            mqttClient.publish(mqttLightStateTopic, (char *)(&mqttStateTopicVal));
            mqttLightStateTopic[idPosition] = 0;
          }
        }
        break;
      case MQTT_PUBLISH_BUTTON_STATE:
        idPosition = setupTopicForItem(mqttButtonTopic, 0, data.item + 1);
        mqttStateTopicVal = '0' + data.state;
        mqttClient.publish(mqttButtonTopic, (char *)(&mqttStateTopicVal));
        mqttButtonTopic[idPosition] = 0;
        break;
      default:
        break;
    }
  }

  mqttClient.loop();
}
