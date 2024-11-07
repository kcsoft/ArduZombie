#include <Ethernet.h>
#include "config.h"
#include "dhcp.h"
#include "status.h"
#include "settings.h"

uint8_t _dhcpStatus;
uint8_t mac[6] = {0x00, 0x08, 0xDC, 0x1D, 0x62, 0x00};

void setupMAC() {
  mac[5] = settings.id; // TODO: read settings from EEPROM
}

void printIP() {
  DEBUG_PRINT("IP: ");
  for (uint8_t i = 0; i < 4; i++) {
    DEBUG_PRINTDEC(Ethernet.localIP()[i]);
    DEBUG_PRINT('.');
  }
  DEBUG_PRINTLN();
}
void dhcpBegin() {
  setupMAC();
  _dhcpStatus = Ethernet.begin(mac); // 1 = success, 0 = fail
  if (_dhcpStatus == 1) {
    setStatus(STATUS_DHCP_CONNECTED);
    printIP();
  } else {
    setStatus(STATUS_INIT);
  }
}

void dhcpSetup() {
  dhcpBegin();
}

void dhcpLoop() {
  if (_dhcpStatus == 0) {
    dhcpBegin();
  }

  uint8_t ethStatus = Ethernet.maintain();
  if (ethStatus == DHCP_CHECK_RENEW_OK || ethStatus == DHCP_CHECK_REBIND_OK) {
    printIP();
  } else if (ethStatus == DHCP_CHECK_RENEW_FAIL || ethStatus == DHCP_CHECK_REBIND_FAIL) {
    _dhcpStatus = 0;
  }
}
