/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Media.h
 *
 ****/

#pragma once

#include "Channel.h"

namespace GoogleCast
{
class Media : public Channel
{
public:
	enum class State {
		NoChange,
		Play,
		Pause,
	};

	using Channel::Channel;

	String getNameSpace() const override
	{
		return F("urn:x-cast:com.google.cast.media");
	}

	bool load(const Url& url, MimeType mime)
	{
		return load(url, toString(mime));
	}

	bool load(const Url& url, const String& mime)
	{
		return load(url.toString(), mime);
	}

	bool load(const String& url, const String& mime);

	bool pause(int sessionId);
	bool play(int sessionId);
	bool stop(int sessionId);

	bool seek(int sessionId, float currentTime, State newState = State::NoChange);
};

} // namespace GoogleCast