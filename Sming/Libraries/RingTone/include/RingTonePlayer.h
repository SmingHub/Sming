/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Player.h - Support for playing simple ring tones
 *
 * RTTTL conversion code based on https://github.com/end2endzone/NonBlockingRTTTL
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <Timer.h>
#include <Delegate.h>
#include <Platform/Timers.h>
#include "RtttlParser.h"

namespace RingTone
{
/**
 * @brief Base class to support playback of tunes
 * @note This class doesn't produce any sound. To do this provide set the
 * `onPrepareNote` and `onPlayNote` callbacks, or override the
 * `prepareNote` and `playNote` methods in an inherited class.
 */
class Player
{
public:
	using PrepareNoteDelegate = Delegate<void(unsigned frequency)>;
	using CompleteDelegate = Delegate<void()>;
	using PlayNoteDelegate = Delegate<void()>;

	Player() : totalPlayTime(NanoTime::Milliseconds, 0)
	{
	}

	~Player()
	{
		end();
	}

	/**
	 * @brief Initialise player
	 * @param parser The source of ringtone data
	 * @note We don't own the parser, just take a reference
	 */
	void begin(RingTone::Parser* parser);

	/**
	 * @brief Stop the player and un-reference parser
	 */
	void end();

	/**
	 * @brief Start or continue playing the tune
	 */
	bool start(unsigned delayMs = 0);

	/**
	 * @brief Stop/pause playing the tune
	 */
	void stop();

	/**
	 * @brief Stop playback and reset play time
	 */
	void resetPlayTime()
	{
		totalPlayTime.time = 0;
		playTime.start();
	}

	/**
	 * @brief Determine if a tune is being played
	 */
	bool isStarted()
	{
		return started;
	}

	void onPrepareNote(PrepareNoteDelegate delegate)
	{
		prepareNoteDelegate = delegate;
	}

	void onPlayNote(PlayNoteDelegate delegate)
	{
		playNoteDelegate = delegate;
	}

	void onComplete(CompleteDelegate delegate)
	{
		completeDelegate = delegate;
	}

	const NanoTime::Time<uint32_t> getPlayTime()
	{
		auto time = totalPlayTime;
		if(started) {
			time += playTime.elapsedTime();
		}
		return time;
	}

	/**
	 * @brief Get playback speed factor
	 * @retval unsigned 100 = normal speed, 50 = half speed, 200 = double speed, etc.
	 */
	unsigned getSpeed() const
	{
		return speed;
	}

	/**
	 * @brief Set playback speed factor
	 * @retval unsigned New speed factor
	 */
	unsigned setSpeed(unsigned speed)
	{
		this->speed = std::min(std::max(speed, 10U), 250U);
		return this->speed;
	}

	/**
	 * @brief Make a relative adjustment to playback speed
	 * @retval unsigned New speed factor
	 */
	unsigned adjustSpeed(int adjust)
	{
		return setSpeed(speed + adjust);
	}

protected:
	/**
	 * @brief Prepare a tone to be played
	 * @frequency Frequency of tone, in Hz. 0 means 'mute'
	 */
	virtual void prepareNote(unsigned frequency)
	{
		if(prepareNoteDelegate) {
			prepareNoteDelegate(frequency);
		}
	}

	/**
	 * @brief Play a prepared tone
	 */
	virtual void playNote()
	{
		if(playNoteDelegate) {
			playNoteDelegate();
		}
	}

	/**
	 * @brief Override this method to be called when a tune has finished playing
	 * @note This doesn't get called if manually stopped
	 */
	virtual void complete()
	{
		if(completeDelegate) {
			completeDelegate();
		}
	}

private:
	void scaleNote()
	{
		preparedNote.duration = (preparedNote.duration * 100) / speed;
	}

	void playPreparedNote();

private:
	RingTone::Parser* parser = nullptr;
	PrepareNoteDelegate prepareNoteDelegate;
	PlayNoteDelegate playNoteDelegate;
	CompleteDelegate completeDelegate;
	NanoTime::Time<uint32_t> totalPlayTime; ///< Marks elapsed time if playback paused
	OneShotFastMs playTime;
	Timer noteTimer;
	bool started = false;
	uint8_t speed = 100;
	RingTone::NoteDef preparedNote; ///< Prepared note to be played
};

} // namespace RingTone
