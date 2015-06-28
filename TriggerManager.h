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


//  +-------+------------------------+-------+---------+-------------+---------------+
//  | Bez 1 |          Bez2          | Bez3  | Pin 595 | Pin Arduino | Atmega Intern |
//  +-------+------------------------+-------+---------+-------------+---------------+
//  | SER   | Serial Data Input      | DS    |      14 |          11 | B,3           |
//  | RCK   | Register Clock Pin     | SH_CP |      12 |           8 | B,0           |
//  | SRCK  | Storage Register Clock | ST_CP |      11 |          12 | B,4           |
//  +-------+------------------------+-------+---------+-------------+---------------+

#define SHIFTREGISTER_SER B,3  // Serial Data Input DS, 11
#define SHIFTREGISTER_RCK B,0  // Register Clock Pin (SH_CP) ,12
#define SHIFTREGISTER_SRCK B,4 // Storage Register Clock ST_CP, 8

#include "shiftRegisterFast.h"


// the number of 595s that are daisy chained
// could be calculated from number of channels but using a constant avoids dynamic allocation
// can be reworked to a template to avoid this definition
const uint8_t number595 = 3;

// defines the time unit in which you are setting delay and hold time as 1/interruptFreq
// value can be between 15626 and 62
// 4k seems to be kind of an upper limit. To go higher you need to carefully measure timings.
const uint16_t interruptFreq = 3000;


class TriggerManager {
public:
	TriggerManager() {}
	~TriggerManager() {}

	// initialize and set all needed timing information
	void init(uint8_t numberChannels, uint8_t* preDelay, uint8_t* holdTime);

	// set pre delay for an individual channel
	void setPreDelay(uint8_t channel, uint8_t time);

	// set hold time for an individual channel
	void setHoldTime(uint8_t channel, uint8_t time);

	// start the trigger process for a given channel
	void setOn(uint8_t channel /*,uint8_t velocity*/);
	/*void setOff(uint8_t channel);*/
	/*void setInverse(uint8_t channel, bool val); */

	// set all triggers off immediately
	void setAllOff();


// for ISR; not actually public
public:
	void checkForToggle();

private:
	volatile uint8_t states[number595];
	volatile uint8_t toggleTimes[number595*8];
	uint8_t preDelays[number595*8];
	uint8_t holdTimes[number595*8];

	void update();


	void toggleChannel(uint8_t number);
	bool isChannelOn(uint8_t number);



};



#endif /* TRIGGERMANAGER_H_ */
