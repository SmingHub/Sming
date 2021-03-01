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
	using Channel::Channel;

	String getNameSpace() const override
	{
		return F("urn:x-cast:com.google.cast.media");
	}

	bool load(const Url& url, MimeType mime)
	{
		return load(url, toString(mime));
	}

	bool load(const Url& url, const String& mime);

	bool pause(const String& sessionId);
	bool play(const String& sessionId);
	bool stop(const String& sessionId);
};

} // namespace GoogleCast