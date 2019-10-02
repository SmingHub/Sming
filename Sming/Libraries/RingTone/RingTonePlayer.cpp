/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Player.cpp
 *
 ****/

#undef DEBUG_VERBOSE_LEVEL
#define DEBUG_VERBOSE_LEVEL 0

#include "include/RingTonePlayer.h"
#include <ctype.h>
#include <debug_progmem.h>

namespace RingTone
{
void Player::begin(RingTone::Parser* parser)
{
	end();
	this->parser = parser;
}

void Player::end()
{
	stop();
	resetPlayTime();
	parser = nullptr;
}

void Player::playPreparedNote()
{
	if(preparedNote.duration == 0) {
		// End of tune
		stop();
		complete();
		return;
	}

	playNote();
	noteTimer.setIntervalMs(preparedNote.duration);
	noteTimer.startOnce();
	if(preparedNote.frequency == 0) {
		debug_i("Pausing: %u", preparedNote.duration);
	} else {
		debug_i("Playing: %u %u (%u Hz) %u ms", preparedNote.scale, preparedNote.note, preparedNote.frequency,
				preparedNote.duration);
	}

	if(parser->readNextNote(preparedNote)) {
		scaleNote();
		prepareNote(preparedNote.frequency);
	} else {
		preparedNote.duration = 0;
	}
}

bool Player::start(unsigned delayMs)
{
	if(started) {
		debug_i("Already playing");
		return true;
	}

	if(parser == nullptr) {
		debug_e("Not initialised");
		return false;
	}

	if(!parser->readNextNote(preparedNote)) {
		complete();
		return false;
	}

	scaleNote();
	prepareNote(preparedNote.frequency);
	noteTimer.setCallback(TimerDelegate(&Player::playPreparedNote, this));
	noteTimer.setIntervalMs(std::max(delayMs, 100U));
	noteTimer.startOnce();
	playTime.start();
	started = true;
	return true;
}

void Player::stop()
{
	if(started) {
		noteTimer.stop();
		totalPlayTime += playTime.elapsedTime();
		prepareNote(0);
		playNote();
		started = false;
	}
}

} // namespace RingTone
