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
		RtttlParserState curState = *this;
		unsigned curPos = buffer.getPos();

		if(seekTune(0)) {
			unsigned count = 0;
			do {
				++count;
			} while(nextTune());

			buffer.setPos(curPos);
			*static_cast<RtttlParserState*>(this) = curState;

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

	defaultNoteDuration = 4;
	defaultOctave = 5;

	// Title
	title.setLength(0);
	title.reserve(64);
	char c;
	while((c = buffer.readChar()) != ':') {
		title += c;
	}
	skipWhitespace();

	// format: d=N,o=N,b=NNN:

	// get default duration
	c = buffer.peekChar();
	if(c == 'd') {
		buffer.readChar(); // d
		buffer.readChar(); // =
		unsigned num = readNum();
		if(num != 0) {
			defaultNoteDuration = num;
		}

		buffer.readChar(); // skip ,
		skipWhitespace();
		c = buffer.peekChar();
	}

	debug_i("default duration: %u", defaultNoteDuration);

	// Default octave
	if(c == 'o') {
		buffer.readChar(); // o
		buffer.readChar(); // =
		unsigned num = readNum();
		if(num >= 3 && num <= 7) {
			defaultOctave = num;
		}

		buffer.readChar(); // skip ,
		skipWhitespace();
		c = buffer.peekChar();
	}

	debug_i("default octave: %u", defaultOctave);

	// Beats Per Minute
	unsigned bpm;
	if(c == 'b') {
		buffer.readChar(); // b
		buffer.readChar(); // =
		bpm = readNum();

		buffer.readChar(); // skip :
		skipWhitespace();
	} else {
		bpm = 63;
	}

	debug_i("bpm: %u", bpm);

	// BPM expresses the number of quarter notes per minute
	wholeNoteMillis = 4UL * 60UL * 1000UL / bpm;
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
	unsigned duration = wholeNoteMillis / (readNum() ?: defaultNoteDuration);

	// Note
	unsigned noteNumber = RingTone::charToNoteNumber(buffer.readChar());

	// Optional '#' sharp
	char c = buffer.peekChar();
	if(c == '#') {
		if(noteNumber != 0) {
			noteNumber += 1;
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
		scale = defaultOctave;
	}

	// Skip to next note (or end of tune)
	if(c == ',') {
		buffer.readChar();
	}

	note.frequency = RingTone::getNoteFrequency(scale, noteNumber);
	note.duration = duration;

	return true;
}
