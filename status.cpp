#include <Arduino.h>

#include "config.h"
#include "status.h"

enum boardStatus status;

uint8_t statusCounter;
#define PAUSE_LOOPS 3

void setStatus(uint8_t newStatus) {
  status = newStatus;
  statusCounter = 0;
}

uint8_t getStatus() {
  return status;
}

void statusSetup() {
  status = STATUS_INIT;
  pinMode(LED_BUILTIN, OUTPUT);
}

// called every 100ms
void statusLoop() {
  if (++statusCounter > (2 * status + PAUSE_LOOPS)) {
    statusCounter = 0;
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    if (statusCounter > (2 * status)) {
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      digitalWrite(LED_BUILTIN, statusCounter % 2 ? HIGH : LOW);
    }
  }
}
