/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AudioPlayer.h
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <RingTonePlayer.h>
#include <Services/Profiling/MinMaxTimes.h>
#include <ToneGenerator.h>

#define AP_STATE_MAP(XX)                                                                                               \
	XX(Stopped)                                                                                                        \
	XX(Paused)                                                                                                         \
	XX(Started)

#define AP_PLAY_MODE_MAP(XX)                                                                                           \
	XX(Sequential)                                                                                                     \
	XX(Random)

#define AP_NOTIFY_CODE_MAP(XX)                                                                                         \
	XX(Info)                                                                                                           \
	XX(Warning)                                                                                                        \
	XX(Error)                                                                                                          \
	XX(TuneChanged)                                                                                                    \
	XX(TuneCompleted)                                                                                                  \
	XX(Started)                                                                                                        \
	XX(Paused)                                                                                                         \
	XX(Stopped)                                                                                                        \
	XX(VoiceChanged)                                                                                                   \
	XX(ModeChanged)                                                                                                    \
	XX(SpeedChanged)

class AudioPlayer
{
public:
	enum class State {
#define XX(t) t,
		AP_STATE_MAP(XX)
#undef XX
			MAX
	};

	enum class PlayMode {
#define XX(t) t,
		AP_PLAY_MODE_MAP(XX)
#undef XX
			MAX
	};

	using Voice = ToneGenerator::Voice;

	enum class NotifyCode {
#define XX(t) t,
		AP_NOTIFY_CODE_MAP(XX)
#undef XX
	};

	using NotifyDelegate = Delegate<void(NotifyCode code, const String& msg)>;

	AudioPlayer() : fillTimes("Fill Times")
	{
	}

	bool begin(IDataSourceStream* tunesFile, unsigned sampleRate);

	bool playNextTune();

	bool playTune(unsigned index);

	bool playPreviousTune();

	bool skipTune();

	unsigned getIndex()
	{
		return parser.getIndex();
	}

	String getCaption()
	{
		return parser.getCaption();
	}

	const NanoTime::Time<uint32_t> getPlayTime()
	{
		return ringtone.getPlayTime();
	}

	State getState()
	{
		if(toneGenerator.isStarted()) {
			return ringtone.isStarted() ? State::Started : State::Paused;
		} else {
			return State::Stopped;
		}
	}

	bool isStarted()
	{
		return getState() == State::Started;
	}

	bool start(unsigned delayMs = 0)
	{
		if(!toneGenerator.start()) {
			return false;
		}
		if(!ringtone.isStarted()) {
			if(!ringtone.start(delayMs)) {
				return false;
			}
			notify(NotifyCode::Started);
		}
		return true;
	}

	void stop()
	{
		if(toneGenerator.isStarted()) {
			ringtone.stop();
			toneGenerator.stop();
			notify(NotifyCode::Stopped);
		}
	}

	void pause()
	{
		if(ringtone.isStarted()) {
			ringtone.stop();
		} else if(!toneGenerator.isStarted()) {
			toneGenerator.start();
		} else {
			return; // no change
		}
		notify(NotifyCode::Paused);
	}

	bool rewind()
	{
		pause();
		return parser.rewind();
	}

	Voice getVoice()
	{
		return voice;
	}

	Voice setVoice(Voice voice)
	{
		if(voice != this->voice) {
			this->voice = voice;
			notify(NotifyCode::VoiceChanged, getVoiceName());
		}
		return this->voice;
	}

	Voice nextVoice()
	{
		return setVoice(Voice((unsigned(voice) + 1) % unsigned(Voice::MAX)));
	}

	String getVoiceName()
	{
		return toneGenerator.getVoiceName(voice);
	}

	PlayMode getMode()
	{
		return playMode;
	}

	PlayMode setMode(PlayMode mode)
	{
		if(playMode != mode) {
			playMode = mode;
			notify(NotifyCode::ModeChanged, getModeName());
		}
		return playMode;
	}

	PlayMode nextMode()
	{
		return setMode(PlayMode((unsigned(playMode) + 1) % unsigned(PlayMode::MAX)));
	}

	static String getModeName(PlayMode mode);

	String getModeName()
	{
		return getModeName(playMode);
	}

	unsigned getSpeed() const
	{
		return ringtone.getSpeed();
	}

	unsigned setSpeed(unsigned speed)
	{
		auto prev = getSpeed();
		speed = ringtone.setSpeed(speed);
		if(speed != prev) {
			notify(NotifyCode::SpeedChanged, String(speed));
		}
		return speed;
	}

	unsigned adjustSpeed(int adj)
	{
		auto prev = getSpeed();
		auto speed = ringtone.adjustSpeed(adj);
		if(speed != prev) {
			notify(NotifyCode::SpeedChanged, String(speed));
		}
		return speed;
	}

	static String getNotifyCodeName(NotifyCode code);

	void onNotify(NotifyDelegate delegate)
	{
		notifyDelegate = delegate;
	}

	// Track processing time
	Profiling::CpuCycleTimes fillTimes;

protected:
	bool queueTune();
	bool tuneComplete();

	void notify(NotifyCode code, const String& msg = nullptr)
	{
		if(notifyDelegate) {
			notifyDelegate(code, msg);
		}
	}

private:
	RingTone::RtttlParser parser;
	RingTone::Player ringtone;
	ToneGenerator toneGenerator;
	Voice voice = Voice::Sine;
	PlayMode playMode = PlayMode::Sequential;
	NotifyDelegate notifyDelegate;
};
