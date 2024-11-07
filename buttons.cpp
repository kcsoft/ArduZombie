#include <Arduino.h>

#include "config.h"
#include "actions.h"
#include "buttons.h"
#include "settings.h"

uint8_t buttonState[BUTTONS / 8]; // the inputs PINA, PINC
uint16_t buttonDebounce[BUTTONS];

uint8_t timer1CountReadButtons;
uint8_t readButtonsPortIndex, readButtonsButtonIndex, readButtonsMask, readButtonsBit;
uint8_t action, param;

// called from T1 ISR every 1ms
void buttonsRead() {
  if (++timer1CountReadButtons >= 5) {
    if (timer1CountReadButtons == 5) { // = 5 aquire inputs
      DDRA = 0;
      DDRC = 0;
      PORTA = 0xFF;
      PORTC = 0xFF;
      delayMicroseconds(40);

      readButtonsPortIndex = 0;
      readButtonsButtonIndex = 0;
      buttonState[0] = PINA;
      buttonState[1] = PINC;
      PORTA = 0;
      PORTC = 0;
    } else { // timer1CountReadButtons > 5, process inputs
      timer1CountReadButtons = 0;
      while (readButtonsPortIndex < (BUTTONS / 8)) {
        readButtonsMask = 1;
        readButtonsBit = 0;
        while (readButtonsBit < 8) {
          if ((buttonState[readButtonsPortIndex] & readButtonsMask) == LOW) { // button pressed
            if (buttonDebounce[readButtonsButtonIndex] < DEBOUNCE) {
              if (++buttonDebounce[readButtonsButtonIndex] >= DEBOUNCE) { // button push event
                action = settings.buttonShortActions[readButtonsButtonIndex] >> 4;
                param = settings.buttonShortActions[readButtonsButtonIndex] & 0x0F;
                executeAction(action, param);
              }
            } else
            // check if long press
            if (buttonDebounce[readButtonsButtonIndex] < DEBOUNCE_LONG) {
              if (++buttonDebounce[readButtonsButtonIndex] >= DEBOUNCE_LONG) { // button long press
                action = settings.buttonLongActions[readButtonsButtonIndex] >> 4;
                param = settings.buttonLongActions[readButtonsButtonIndex] & 0x0F;
                executeAction(action, param);
              }
            }
          } else {
            // check if medium press
            if (
              buttonDebounce[readButtonsButtonIndex] >= DEBOUNCE_MEDIUM
              && buttonDebounce[readButtonsButtonIndex] < DEBOUNCE_LONG
            ) { // button medium press
              action = settings.buttonMediumActions[readButtonsButtonIndex] >> 4;
              param = settings.buttonMediumActions[readButtonsButtonIndex] & 0x0F;
              executeAction(action, param);
            }
            buttonDebounce[readButtonsButtonIndex] = 0;
          }
          readButtonsButtonIndex++;
          readButtonsMask <<= 1;
          readButtonsBit++;
        }
        readButtonsPortIndex++;
      }
    }
  }
}
