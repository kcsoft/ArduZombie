#include <avr/wdt.h>
/*
Libraries:
- Ethernet by Arduino v2.0.2
- PubSubClient by Nick O'Leary v2.8.0 https://github.com/knolleary/pubsubclient
- CircularBuffer by AgileWare v1.4.0 https://github.com/rlogiacco/CircularBuffer
*/

#include "config.h"
#include "settings.h"
#include "dhcp.h"
#include "mqtt.h"
#include "blink.h"
#include "buttons.h"
#include "lights.h"
#include "status.h"

uint16_t isrCounter = 0;

void setup() {
  DEBUG_INIT();

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 2000; // 10000*8*0.0625=5ms 2000=1ms
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS11); // 8 prescaler
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt

  PORTA = 0;
  PORTC = 0;

  loadSettings();
  statusSetup();
  interrupts();

  lightsSetup();
  setBlinkMode(4);
  dhcpSetup();
  mqttSetup();

  wdt_enable(WDTO_2S);
}

void loop() {
  wdt_reset();
  dhcpLoop();
  mqttLoop();
  statusLoop();
  delay(100);
}

// every 1ms
ISR(TIMER1_COMPA_vect) {
  buttonsRead();
  blink();
  if (++isrCounter >= 1000) {
    isrCounter = 0;
    lightsLoop();
  }
}
