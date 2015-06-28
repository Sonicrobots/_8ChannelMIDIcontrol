// 
// /~~\ /~~\ |\  |~|~/~~  |~~\ /~~\ |~~\ /~~\~~|~~/~~\
// `--.|    || \ | ||     |__/|    ||--<|    | |  `--.
// \__/ \__/ |  \|_|_\__  |  \ \__/ |__/ \__/  |  \__/
//
// 8-Channel- Midi Out with Latenz and trigger control
// === Recources: ===
// MIDI Library https://github.com/FortySevenEffects/arduino_midi_library (used: c4.2.)
// 
// 
//Die Out Pins sind:
//
//PB0
//PB1
//PB2
//PB3
//PB4
//
//PC1
//
//PD5
//PD6

// does not get parsed when using Arduino IDE
#ifdef EXTERNAL_IDE
#include <Arduino.h>
#include <shiftRegisterFast.h>
int main(void) {
  init();
  setup();
  while(true) {
    loop();
  }
}
#endif


#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>
MIDI_CREATE_DEFAULT_INSTANCE();


#include "TriggerManager.h"
extern TriggerManager triggers;

//  Set up dynamic behavior for different 595 pins that are controlled by a TriggerManager
const uint8_t channels = 25;
 //             	595-PIN    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
uint8_t preDelays[channels] = {0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  };
uint8_t holdTimes[channels] = {90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90  };
uint8_t  midiNote[channels] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25  };

const uint8_t testLED = 13;


//#define DEBUG




// Readout the coded switch on PIN 2,4,5,7 on Startup. To set the MIDI Channel (1-16)
uint8_t readMidiChannel()
{
	// set pins as inputs
	pinMode(2, INPUT_PULLUP);pinMode(7, INPUT_PULLUP);pinMode(4, INPUT_PULLUP);pinMode(5, INPUT_PULLUP); // the pins for the coded channel switch with build in PULLUP resistors

	uint8_t i=0;
	bitWrite(i, 0, !digitalRead(2));
	bitWrite(i, 1, !digitalRead(5));
	bitWrite(i, 2, !digitalRead(7));
	bitWrite(i, 3, !digitalRead(4));

	return i+1; //+1 .. nullausgleich
}


void HandleNoteOn(byte channel, byte note, byte velocity) {
			digitalWrite(testLED,HIGH);
	// searching for the channel that fits the received note
    for (uint8_t index=0; index< channels; index++) {

		if (midiNote[index] == note) {


			triggers.setOn(note);

			return; // no need to search further
		}
	}
}

void HandleNoteOff(byte channel, byte note, byte velocity) {

	// just for indication if midi works
	digitalWrite(testLED,LOW);

}




void setup() {
      pinMode(testLED, OUTPUT);

      #ifndef DEBUG
      MIDI.setHandleNoteOn(HandleNoteOn);
      MIDI.setHandleNoteOff(HandleNoteOff);
      MIDI.begin(readMidiChannel());  // listens on only channel which is set up with the coded switch
	  #endif

	  #ifdef DEBUG
      Serial.begin(115200);
      Serial.println("Hello");
	  #endif

      triggers.init(channels,&preDelays[0],&holdTimes[0]);


}


void loop() {
	#ifndef DEBUG
	MIDI.read();
	#endif

    #ifdef DEBUG
	uint8_t note = Serial.parseInt(); // this waits until a number is received

	if (note != 0) { // 0 has to be excluded because it is returned as a timeout in parseInt()
		Serial.print("On "); Serial.println(note);
		HandleNoteOn(0,note,255);
	}
	#endif


	// set the hold time for all channels
	uint8_t holdTime = analogRead(4);

	static uint8_t oldHoldTime;
	if (holdTime != oldHoldTime) {
		oldHoldTime = holdTime;

		#ifdef DEBUG
		Serial.print("Hold time changed: "); Serial.println(holdTime);
		#endif

		for (uint8_t index=0; index<channels; index++) {
			triggers.setHoldTime(index,holdTime);
		}
	}





    
    
    
}


