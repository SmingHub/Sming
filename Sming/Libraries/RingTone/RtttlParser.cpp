/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RtttlParser.cpp
 *
 ****/

// Comment this out to emit debug statements
#undef DEBUG_VERBOSE_LEVEL
#define DEBUG_VERBOSE_LEVEL 0

#include "include/RtttlParser.h"
#include "include/RingTonePlayer.h"
#include <ctype.h>
#include <debug_progmem.h>

using namespace RingTone;

static bool iscomment(char c)
{
	return c == '#' || c == ';';
}

bool RtttlParser::begin(IDataSourceStream* source)
{
	if(source == nullptr) {
		return false;
	}

	if(!source->isValid()) {
		delete source;
		return false;
	}

	int len = source->available();
	if(len < 0) {
		delete source;
		return false;
	}

	buffer.setStream(source);
	state = rtps_StartOfFile;
	count = 0;
	index = 0;
	return nextTune();
}

void RtttlParser::end()
{
	buffer.setStream(nullptr);
	count = 0;
	index = 0;
}

unsigned RtttlParser::readNum()
{
	unsigned num = 0;
	while(true) {
		char c = buffer.peekChar();
		if(!isdigit(c)) {
			break;
		}
		num = (num * 10) + c - '0';
		buffer.readChar();
	}
	return num;
}

bool RtttlParser::skipWhitespace()
{
	bool inComment = false;
	while(!buffer.eof()) {
		auto c = buffer.peekChar();

		if(c != '\n') {
			if(state == rtps_StartOfFile && iscomment(c)) {
				inComment = true;
			}

			if(inComment || isspace(c)) {
				buffer.readChar();
				continue;
			}

			if(iscomment(c)) {
				inComment = true;
				buffer.readChar();
				continue;
			}

			return true;
		}

		buffer.readChar();
		c = buffer.peekChar();

		if(c == '\r' || c == '\n') {
			continue;
		}

		if(iscomment(c)) {
			inComment = true;
			continue;
		}

		if(!isblank(c)) {
			if(state != rtps_StartOfFile) {
				++index;
			}
			state = rtps_TuneHeader;
			return true;
		}

		inComment = false;
	}

	state = rtps_EndOfFile;
	return false;
}

bool RtttlParser::nextTune()
{
	if(state == rtps_TuneContent) {
		// Skip rest of tune
		while(!buffer.eof()) {
			auto c = buffer.peekChar();

			// Tune starts on new line with non-comment, non-whitespace
			if(c != '\n') {
				buffer.readChar();
				continue;
			}

			skipWhitespace();

			if(state == rtps_TuneHeader) {
				break;
			}
		}
	}

	return readHeader();
}

unsigned RtttlParser::getCount()
{
	if(count == 0) {
		auto curState = getState();

		if(seekTune(0)) {
			unsigned count = 0;
			do {
				++count;
			} while(nextTune());

			setState(curState);

			this->count = count;
		}
	}

	return count;
}

bool RtttlParser::seekTune(unsigned index)
{
	if(!buffer.setPos(0)) {
		return false;
	}

	state = rtps_StartOfFile;
	this->index = 0;

	while(nextTune()) {
		if(this->index == index) {
			return true;
		}
	}

	return false;
}

bool RtttlParser::readHeader()
{
	if(!skipWhitespace()) {
		return false;
	}

	header.defaultNoteDuration = 4;
	header.defaultOctave = 5;

	// Title
	header.title.setLength(0);
	header.title.reserve(64);
	char c;
	while((c = buffer.readChar()) != ':') {
		header.title += c;
	}
	header.title.trim();
	skipWhitespace();

	// format: d=N,o=N,b=NNN:

	// get default duration
	c = buffer.peekChar();
	if(c == 'd') {
		buffer.readChar(); // d
		buffer.readChar(); // =
		unsigned num = readNum();
		if(num != 0) {
			header.defaultNoteDuration = num;
		}

		buffer.readChar(); // skip ,
		skipWhitespace();
		c = buffer.peekChar();
	}

	debug_i("default duration: %u", header.defaultNoteDuration);

	// Default octave
	if(c == 'o') {
		buffer.readChar(); // o
		buffer.readChar(); // =
		unsigned num = readNum();
		if(num >= 3 && num <= 7) {
			header.defaultOctave = num;
		}

		buffer.readChar(); // skip ,
		skipWhitespace();
		c = buffer.peekChar();
	}

	debug_i("default octave: %u", header.defaultOctave);

	// Beats Per Minute
	if(c == 'b') {
		buffer.readChar(); // b
		buffer.readChar(); // =
		header.bpm = readNum();

		buffer.readChar(); // skip :
		skipWhitespace();
	} else {
		header.bpm = 63;
	}

	debug_i("bpm: %u", header.bpm);

	// BPM expresses the number of quarter notes per minute
	wholeNoteMillis = 4U * 60U * 1000U / header.bpm;
	debug_i("wholenote: %u ms", wholeNoteMillis);

	tuneStartPos = buffer.getPos();
	state = rtps_TuneContent;
	return true;
}

bool RtttlParser::readNextNote(RingTone::NoteDef& note)
{
	skipWhitespace();
	if(state != rtps_TuneContent) {
		return false;
	}

	// Get note duration, use default if not provided
	unsigned duration = readNum() ?: header.defaultNoteDuration;
	duration = muldiv(uint32_t(wholeNoteMillis), uint32_t(1), duration);

	// Note
	unsigned noteValue = RingTone::charToNoteValue(buffer.readChar());

	// Optional '#' sharp
	char c = buffer.peekChar();
	if(c == '#') {
		if(noteValue != 0) {
			noteValue += 1;
		}
		buffer.readChar();
		c = buffer.peekChar();
	}

	// Optional '.' dotted note
	if(c == '.') {
		duration += duration / 2;
		buffer.readChar();
		c = buffer.peekChar();
	}

	// Scale
	unsigned scale;
	if(isdigit(c)) {
		scale = c - '0';
		buffer.readChar();
		c = buffer.peekChar();
	} else {
		scale = header.defaultOctave;
	}

	// Skip to next note (or end of tune)
	if(c == ',') {
		buffer.readChar();
	}

	note.frequency = RingTone::getNoteFrequency(scale, noteValue);
	note.duration = duration;

	return true;
}
