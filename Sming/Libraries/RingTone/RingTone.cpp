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

unsigned charToNoteValue(char c)
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

unsigned getNoteFrequency(unsigned octave, unsigned noteValue)
{
	if(noteValue == 0) {
		return 0;
	}

	--noteValue;

	if(noteValue >= 12) {
		noteValue += octave * 12;
		octave = noteValue / 12;
		noteValue %= 12;
	}

	unsigned freq = pgm_read_word(&frequencyTable[noteValue]);

	if(octave < 4) {
		freq >>= 4 - octave;
	} else if(octave > 4) {
		freq <<= octave - 4;
	}

	return (freq + SCALE_FACTOR / 2) / SCALE_FACTOR;
}

unsigned getClosestNote(unsigned frequency, unsigned& octave)
{
	octave = 0;
	if(frequency == 0) {
		return 0;
	}

	// https://newt.phys.unsw.edu.au/jw/notes.html
	constexpr double C4Freq = 261.63;
	int semitones = (4 * 12) + round(12 * log2(frequency / C4Freq));
	if(semitones <= 0) {
		return 0;
	}

	octave = semitones / 12;
	return 1 + (semitones % 12);
}

const char* getNoteName(unsigned noteValue)
{
	const char* noteNames[12] = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};
	if(noteValue == 0) {
		return "p";
	}
	--noteValue;
	if(noteValue >= 12) {
		noteValue %= 12;
	}
	return noteNames[noteValue];
}

}; // namespace RingTone
