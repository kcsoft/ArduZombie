#ifndef _CONFIG_H
#define _CONFIG_H

// #define DEBUG               1

#define DEFAULT_ID          '1'
#define DEFAULT_HOSTNAME    "ArduZombie"

// for settings
#define EEPROM_START        100

#define MQTT_HOST           "192.168.1.2"

#define BUTTONS			        16
#define LIGHTS              BUTTONS

// button input pins
#define DEBOUNCE		        3
// 1.5 seconds
#define DEBOUNCE_MEDIUM     255
// 6 seconds
#define DEBOUNCE_LONG       1000


#ifdef DEBUG
  #define DEBUG_INIT()      Serial.begin(115200)
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTDEC(x) Serial.print(x, DEC)
  #define DEBUG_PRINTHEX(x) Serial.print(x, HEX)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_INIT()
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTHEX(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif
