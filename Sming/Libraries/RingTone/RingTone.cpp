/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RingToneParser.cpp
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#include <FakePgmSpace.h>
#include <math.h>
#include "include/RingTone.h"

namespace RingTone
{
// Scale the tabled frequencies for best accuracy whilst keeping within 16 bits
#define SCALE_FACTOR 128
#define SCALE(f) uint16_t(round(SCALE_FACTOR* f))

/*
 * Frequencies for reference octave
 */
static constexpr uint16_t frequencyTable[] PROGMEM = {
	SCALE(261.63), // C4
	SCALE(277.18), // C#4/Db4
	SCALE(293.66), // D4
	SCALE(311.13), // D#4/Eb4
	SCALE(329.63), // E4
	SCALE(349.23), // F4
	SCALE(369.99), // F#4/Gb4
	SCALE(392.00), // G4
	SCALE(415.30), // G#4/Ab4
	SCALE(440.00), // A4
	SCALE(466.16), // A#4/Bb4
	SCALE(493.88), // B4
};

unsigned charToNoteNumber(char c)
{
	switch(tolower(c)) {
	case 'c':
		return 1;
	case 'd':
		return 3;
	case 'e':
		return 5;
	case 'f':
		return 6;
	case 'g':
		return 8;
	case 'a':
		return 10;
	case 'b':
		return 12;
	case 'p':
	default:
		return 0;
	}
}

unsigned getNoteFrequency(unsigned octave, unsigned note)
{
	if(note == 0) {
		return 0;
	}

	--note;

	if(note >= 12) {
		note += octave * 12;
		octave = note / 12;
		note %= 12;
	}

	unsigned freq = pgm_read_word(&frequencyTable[note]);

	if(octave < 4) {
		freq >>= 4 - octave;
	} else if(octave > 4) {
		freq <<= octave - 4;
	}

	return (freq + SCALE_FACTOR / 2) / SCALE_FACTOR;
}

}; // namespace RingTone
