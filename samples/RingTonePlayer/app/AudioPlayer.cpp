/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AudioPlayer.cpp
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "AudioPlayer.h"
#include <Data/CStringArray.h>

constexpr unsigned tuneStartDelayMs = 1000U;

String AudioPlayer::getModeName(PlayMode mode)
{
#define XX(t) #t "\0"
	DEFINE_FSTR_LOCAL(names, AP_PLAY_MODE_MAP(XX));
#undef XX
	return CStringArray(names)[unsigned(mode)];
}

String AudioPlayer::getNotifyCodeName(NotifyCode code)
{
#define XX(t) #t "\0"
	DEFINE_FSTR_LOCAL(names, AP_NOTIFY_CODE_MAP(XX));
#undef XX
	return CStringArray(names)[unsigned(code)];
}

bool AudioPlayer::queueTune()
{
	ringtone.stop();
	ringtone.resetPlayTime();
	notify(NotifyCode::TuneChanged);
	return start(tuneStartDelayMs);
}

bool AudioPlayer::playNextTune()
{
	if(!parser.nextTune()) {
		notify(NotifyCode::Warning, F("No more tunes"));
		pause();
		return false;
	}

	return queueTune();
}

bool AudioPlayer::playTune(unsigned index)
{
	if(isStarted()) {
		if(index == parser.getIndex()) {
			return true;
		}
	}

	if(!parser.seekTune(index)) {
		notify(NotifyCode::Warning, F("Tune #") + String(index) + _F(" not found"));
		pause();
		return false;
	}

	return queueTune();
}

bool AudioPlayer::playPreviousTune()
{
	auto idx = parser.getIndex();
	if(idx == 0) {
		notify(NotifyCode::Warning, F("Already at start"));
		return false;
	}

	return playTune(idx - 1);
}

bool AudioPlayer::skipTune()
{
	notify(NotifyCode::TuneCompleted, getCaption());
	return playTune(1 + (os_random() % parser.getCount()));
}

bool AudioPlayer::tuneComplete()
{
	notify(NotifyCode::TuneCompleted, getCaption());

	switch(playMode) {
	case PlayMode::Random:
		return playTune(1 + (os_random() % parser.getCount()));
	case PlayMode::Sequential:
		return playNextTune();
	default:
		return false;
	}
}

bool AudioPlayer::begin(IDataSourceStream* tunesFile, unsigned sampleRate)
{
	if(!parser.begin(tunesFile)) {
		notify(NotifyCode::Error, F("Opening tunes file"));
		return false;
	}

	ringtone.begin(&parser);

	if(!toneGenerator.begin(sampleRate)) {
		notify(NotifyCode::Error, F("Initialising Tone Generator"));
		return false;
	}

	ringtone.onPrepareNote([this](unsigned frequency) {
		fillTimes.start();
		toneGenerator.queueTone(voice, frequency);
		fillTimes.update();
	});
	ringtone.onPlayNote([this]() { toneGenerator.submitPending(); });
	ringtone.onComplete([this]() { tuneComplete(); });

	toneGenerator.start();
	return true;
}
