/*
 * TriggerManager.cpp
 *
 *  Created on: 11.03.2015
 *      Author: Lennart Schierling for bastl-instruments
 */


#include "TriggerManager.h"
#include "portManipulations.h"

//#define DEBUG_PRINT
//#define DEBUG_IR

#ifdef DEBUG_IR
#define DEBUGPIN D,4
#endif



void TriggerManager::init(PinSettings* pins, uint8_t* preDelay, uint8_t* holdTime) {

  // save arguments
  for (uint8_t index=0; index<numbChannels; index++) {
    setPreDelay(index,preDelay[index]);
    setHoldTime(index,holdTime[index]);
    this->pins[index] = pins[index];
  }

  setAllOff();

  // set pins in output mode
  for (uint8_t index=0; index<numbChannels; index++) {
	  *(this->pins[index].PORT_REG) &= ~(1<<(this->pins[index].PinIndex));
	  *(this->pins[index].DDR_REG) |= 1<<(this->pins[index].PinIndex);
	  states[index] = false;
  }

  //setup timer
  TCCR0A = (1<<WGM01);
  TCCR0B = (1<<CS02) | (1<<CS00);
  OCR0A  = F_CPU/1024/interruptFreq;
  TIMSK0 = (1<<OCIE0A);

  #ifdef DEBUG_IR
  bit_dir_outp(DEBUGPIN);
  bit_set(DEBUGPIN);
  #endif
}

void TriggerManager::setPreDelay(uint8_t channel, uint8_t time) {

	// do nothing if channel number is out of bounds
	if (channel >= numbChannels) return;

	preDelays[channel] = min(254,time);
}
void TriggerManager::setHoldTime(uint8_t channel, uint8_t time) {

	// do nothing if channel number is out of bounds
	if (channel >= numbChannels) return;

	holdTimes[channel] = min(254,time);
}

void TriggerManager::setOn(uint8_t channel) {

  #ifdef DEBUG_PRINT
  Serial.println("checking if valid");
  #endif

  // do nothing if channel number is out of bounds
  if (channel >= numbChannels) return;

  // do nothing if channel is active
  if (toggleTimes[channel] != 255) return;

  #ifdef DEBUG_PRINT
  Serial.println("valid");
  #endif

  toggleTimes[channel] = preDelays[channel];
}

bool TriggerManager::isChannelOn(uint8_t channel) {
  return states[channel];
}


void TriggerManager::checkForToggle() {
  for (uint8_t index=0; index<numbChannels; index++) {

    if (toggleTimes[index] != 255){
      toggleTimes[index]--;

      if (toggleTimes[index] == 255) toggleChannel(index);

    }

  }

}

void TriggerManager::toggleChannel(uint8_t channel) {

  // if this was just the pre-delay, we need to load next trigger for switch off
  if (!isChannelOn(channel)) toggleTimes[channel] = holdTimes[channel];

  // invert state and flip pin
  *(pins[channel].PIN_REG) = 1<<(pins[channel].PinIndex);
  states[channel] = !states[channel];


}

void TriggerManager::setAllOff() {
  for (uint8_t index=0; index<numbChannels; index++) {
    *(pins[index].PORT_REG) &= ~(1<<(pins[index].PinIndex));
    toggleTimes[index] = 255;
    states[index] = 0;

  }

}

TriggerManager triggers;


ISR(TIMER0_COMPA_vect) {
  #ifdef DEBUG_IR
  bit_set(DEBUGPIN);
  #endif
  triggers.checkForToggle();
  #ifdef DEBUG_IR
  bit_clear(DEBUGPIN);
  #endif
}

