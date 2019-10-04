/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RtttlParser.h - Support for reading RTTTL files
 *
 * RTTTL conversion code based on https://github.com/end2endzone/NonBlockingRTTTL
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "StreamBuffer.h"
#include <WString.h>
#include "RingTone.h"

namespace RingTone
{
struct RtttlHeader {
	String title;
	uint8_t defaultNoteDuration = 0; // quarter-note
	uint8_t defaultOctave = 0;
	uint16_t bpm = 0;
};

/*
 * Define core state variables so state can be easily saved/restored
 */
struct RtttlParserState {
	RtttlHeader header;			  // Header for current tune
	uint16_t wholeNoteMillis = 0; // Calculated from bpm
	unsigned count = 0;			  // Cached number of tunes in the file
	unsigned index = 0;
	unsigned tuneStartPos = 0; // First character after header

	enum {
		rtps_StartOfFile,
		rtps_TuneHeader,
		rtps_TuneContent,
		rtps_EndOfFile,
	} state = rtps_StartOfFile;

	// Only used when saving/restoring state
	unsigned bufferPos = 0;
};

/**
 * @brief Class to parse RTTTL files
 * RTTTL (RingTone Text Transfer Language) format
 * @see See https://en.wikipedia.org/wiki/Ring_Tone_Transfer_Language
 */
class RtttlParser : public RingTone::Parser, private RtttlParserState
{
public:
	/**
	 * @brief Initialise the parser with the given stream
	 */
	bool begin(IDataSourceStream* source);

	/**
	 * @brief Release the source stream
	 */
	void end();

	/**
	 * @brief Locate next tune and read header
	 */
	bool nextTune();

	/**
	 * @brief Find a tune by index, starting at #0
	 */
	bool seekTune(unsigned index);

	/**
	 * @brief Get the current tune index
	 */
	unsigned getIndex()
	{
		return index;
	}

	/**
	 * @brief Get the number of tunes in this file
	 */
	unsigned getCount();

	/**
	 * @brief Get the current tune title
	 */
	const String& getTitle()
	{
		return header.title;
	}

	/**
	 * @brief Get a display caption for the current tune
	 */
	String getCaption()
	{
		String s('#');
		s += index;
		s += ": ";
		s += header.title;
		return s;
	}

	/**
	 * @brief Rewind to start of tune
	 */
	bool rewind()
	{
		if(tuneStartPos == 0) {
			// No current tune
			return false;
		}

		return buffer.setPos(tuneStartPos);
	}

	/**
	 * @brief Fetch the next note for this tune
	 * @param note
	 * @retval bool true on success, false if no more notes available
	 */
	bool readNextNote(RingTone::NoteDef& note);

	const RtttlHeader& getHeader()
	{
		return header;
	}

	RtttlParserState getState()
	{
		bufferPos = buffer.getPos();
		return *this;
	}

	void setState(const RtttlParserState& state)
	{
		*static_cast<RtttlParserState*>(this) = state;
		buffer.setPos(bufferPos);
	}

private:
	bool readHeader();
	unsigned readNum();

	// Skip whitespace and comment lines
	bool skipWhitespace();

private:
	StreamBuffer<64> buffer;
};

} // namespace RingTone
