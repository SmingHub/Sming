/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Parser.h - Common definitions for playing ringtones
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <cstdint>
#include <cmath>

namespace RingTone
{
/**
 * @brief Note numbers, defined here for convenience
 */
enum class Note {
	MUTE,
	C,
	C_Sharp,
	D_Flat = C_Sharp,
	D,
	D_Sharp,
	E_Flat = D_Sharp,
	E,
	F,
	F_Sharp,
	G,
	G_Sharp,
	A_Flat = G_Sharp,
	A,
	A_Sharp,
	B_Flat = A_Sharp,
	B,
};

struct NoteDef {
	uint16_t frequency;
	uint16_t duration;
};

/**
 * @brief Reference note frequency
 */
static unsigned noteFrequencyA4 = 440;

static constexpr float frequencyRoot = pow(2, 1.0 / 12);

template <unsigned octave, unsigned note> static constexpr unsigned calculateFrequency()
{
	return round(noteFrequencyA4 * pow(frequencyRoot, (octave - 4) * 12 + (note - 1)));
}

/**
 * @brief Get the corresponding note number for a letter
 * @param c
 * @retval unsigned Notes start at 1, 0 indicates error or pause/mute
 * @note To sharpen a note, add 1
 */
unsigned charToNoteValue(char c);

/**
 * @brief Convert a scale/note combination into frequency
 * @param octave
 * @param note
 * @retval unsigned Frequency, 0 if out of range
 */
unsigned getNoteFrequency(unsigned octave, unsigned note);

/**
 * @brief Convert a frequency into a scale/note combination into frequency
 * @param frequency
 * @param octave Octave for the note
 * @retval unsigned The note number, 0 if out of range
 */
unsigned getClosestNote(unsigned frequency, unsigned& octave);

/**
 * @brief Get text for a given note number
 */
const char* getNoteName(unsigned noteValue);

/**
 * @brief Base parser class
 */
class Parser
{
public:
	virtual ~Parser()
	{
	}

	/**
	 * @brief Fetch the next note for this tune
	 * @retval bool true on success, false if no more notes available
	 */
	virtual bool readNextNote(NoteDef& note) = 0;
};

} // namespace RingTone
