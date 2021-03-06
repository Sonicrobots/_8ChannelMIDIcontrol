// 
// /~~\ /~~\ |\  |~|~/~~  |~~\ /~~\ |~~\ /~~\~~|~~/~~\
// `--.|    || \ | ||     |__/|    ||--<|    | |  `--.
// \__/ \__/ |  \|_|_\__  |  \ \__/ |__/ \__/  |  \__/
//
// 8-Channel- Midi Out with latency and trigger control
// === Recources: ===
// MIDI Library https://github.com/FortySevenEffects/arduino_midi_library (used: c4.2.)
// 
// TOC One Mosfet 
// V.03 


// does not get parsed when using Arduino IDE
#ifdef EXTERNAL_IDE
#include <Arduino.h>

int main(void) {
  init();
  setup();
  while(true) {
    loop();
  }
}
#endif


#include "fastAnalogRead.h"


#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>
MIDI_CREATE_DEFAULT_INSTANCE();


#include "TriggerManager.h"
extern TriggerManager triggers;

static uint8_t holdTime;


#define testLED B,5

//
// Set up dynamic behavior for different 595 pins that are controlled by a TriggerManager
//
const uint8_t channels = 8;
 //              Trigger Pin    0,  1,  2,  3,  4,  5,  6,  7
uint8_t preDelays[channels] = { 0,  1,  1,  1,  0,  0,  0,  0  };
// uint8_t  midiNote[channels] = { 88, 89, 90, 91, 92, 93, 94, 95};
uint8_t  midiNote[channels] = {36, 37,  38,  39,  40,  41,  42,  43};
// --> Neu für Andi C2 ... G2

// Which channel corresponds to which pin on atmega
PinSettings pins[channels] = {
		{0,&DDRB,&PORTB,&PINB},
		{1,&DDRB,&PORTB,&PINB},
		{2,&DDRB,&PORTB,&PINB},
		{3,&DDRB,&PORTB,&PINB},
		{4,&DDRB,&PORTB,&PINB},
		{0,&DDRC,&PORTC,&PINC},
		{5,&DDRD,&PORTD,&PIND},
		{6,&DDRD,&PORTD,&PIND}
};

//#define DEBUG


// Readout the coded switch on PIN 2,3,4,7 on Startup. To set the MIDI Channel (1-16)
uint8_t readMidiChannel()
{
	// set pins as inputs
	pinMode(2, INPUT_PULLUP);pinMode(7, INPUT_PULLUP);pinMode(4, INPUT_PULLUP);pinMode(3, INPUT_PULLUP); // the pins for the coded channel switch with build in PULLUP resistors

	uint8_t i=0;
	bitWrite(i, 0, !digitalRead(3));
	bitWrite(i, 1, !digitalRead(7));
	bitWrite(i, 2, !digitalRead(2));
	bitWrite(i, 3, !digitalRead(4));

	return i+1; //+1 .. nullausgleich
}


void HandleNoteOn(byte channel, byte note, byte velocity) {
	
	bit_set(testLED);

	// searching for the channel that fits the received note
    for (uint8_t index=0; index< channels; index++) {

		if (midiNote[index] == note) {

			#ifdef DEBUG
			Serial.print("= Channel #"); Serial.println(index);
			#endif

			uint8_t thisHoldTime = ((uint16_t)holdTime*velocity)/128;

			triggers.setHoldTime(index,thisHoldTime);
			triggers.setOn(index);

			return; // no need to search further
		}
	}
}

void HandleNoteOff(byte channel, byte note, byte velocity) {

	// just for indication if midi works
	bit_clear(testLED);

}




void setup() {
      
	bit_dir_outp(testLED);
	fastAnalogRead::init();
	fastAnalogRead::connectChannel(4);
	fastAnalogRead::startConversion();

	#ifndef DEBUG
	MIDI.setHandleNoteOn(HandleNoteOn);
	MIDI.setHandleNoteOff(HandleNoteOff);
	MIDI.begin(readMidiChannel());  // listens on only channel which is set up with the coded switch
	#endif
	
	#ifdef DEBUG
	Serial.begin(115200);
	Serial.println("Hello");
	#endif
	
	uint8_t holdTimes[channels] = {90, 90, 90, 90, 90, 90, 90, 90  };
	triggers.init(&pins[0],&preDelays[0],&holdTimes[0]);

}


void loop() {
  #ifndef DEBUG
	MIDI.read();
	#endif

    #ifdef DEBUG
	uint8_t note = Serial.parseInt(); // this waits until a number is received

	if (note != 0) { // 0 has to be excluded because it is returned as a timeout in parseInt()
		Serial.print("Receive Note "); Serial.println(note);
		HandleNoteOn(0,note,255);
	}
	#endif


	// set the hold time for all channels
	if (fastAnalogRead::isConversionFinished()) {
		holdTime = fastAnalogRead::getConversionResult();
		fastAnalogRead::startConversion();
	}
  MIDI.setInputChannel(readMidiChannel());
    
}



