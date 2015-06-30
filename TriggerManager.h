/*
 * TriggerManager.h
 *
 *  Created on: 11.03.2015
 *      Author: Lennart Schierling for bastl-instruments
 *
 *
 *  This object provides an interface that lets you trigger individual pins on daisy-chained 595s.
 *  * For each pin, you can define a pre-delay as well as a hold time.
 *  * Calling setOn() on a particular pin number sets this pin to high after the pre-delay and for the given hold time
 *  * The data pins for the communication with the 595s are set by defines at the beginning of this header file
 *  * Occupies timer 0
 *  * An instance is already created and can be used with extern TriggerManager triggers;
 *
 *
 */

#ifndef TRIGGERMANAGER_H_
#define TRIGGERMANAGER_H_

#include <Arduino.h>



// Struct that has to be used to set pin properties
typedef struct {
	uint8_t   PinIndex;
	volatile uint8_t* DDR_REG;
	volatile uint8_t* PORT_REG;
	volatile uint8_t* PIN_REG;
} PinSettings;


// the total number of channels
// needed for buffer allocation at compile time
const uint8_t NUMCHANNELS = 8;

// defines the time unit in which you are setting delay and hold time as 1/interruptFreq
// value can be between 15626 and 62
// 4k seems to be kind of an upper limit. To go higher you need to carefully measure timings.
const uint16_t interruptFreq = 3000;


class TriggerManager {
public:
	TriggerManager() {}
	~TriggerManager() {}

	// initialize and set pin the pins to be used and all needed timing information
	void init(PinSettings* pins, uint8_t* preDelay, uint8_t* holdTime);

	// set pre delay for an individual channel
	void setPreDelay(uint8_t channel, uint8_t time);

	// set hold time for an individual channel
	void setHoldTime(uint8_t channel, uint8_t time);

	// start the trigger process for a given channel
	void setOn(uint8_t channel /*,uint8_t velocity*/);

	// set all triggers off immediately
	void setAllOff();


// for ISR; not actually public
public:
	void checkForToggle();

private:
	static const uint8_t numbChannels = NUMCHANNELS;

	PinSettings pins[numbChannels]; 			// port information
	volatile bool states[numbChannels];			// on or off (need this to judge if what has to be loaded to countdown
	volatile uint8_t toggleTimes[numbChannels]; // countdown until toggle
	uint8_t preDelays[numbChannels];            // values to be set as countdown when setOn()
	uint8_t holdTimes[numbChannels];            // values to be set as countdown after pin set high

	//void update();


	void toggleChannel(uint8_t number);
	bool isChannelOn(uint8_t number);



};



#endif /* TRIGGERMANAGER_H_ */
