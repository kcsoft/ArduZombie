#ifndef _STATUS_H
#define _STATUS_H

enum boardStatus {
  STATUS_INIT = 1,
  STATUS_DHCP_CONNECTED,
  STATUS_MQTT_CONNECTED
};

void setStatus(uint8_t newStatus);
uint8_t getStatus();

void statusSetup();
void statusLoop();

#endif
