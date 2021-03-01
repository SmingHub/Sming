/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Media.cpp
 *
 ****/

#include "Network/GoogleCast/Media.h"

namespace GoogleCast
{
bool Media::load(const String& url, const String& mime)
{
	StaticJsonDocument<1024> doc;
	initRequest(doc, F("LOAD"));
	doc[F("autoplay")] = true;
	doc[F("currentTime")] = 0;
	auto media = doc.createNestedObject(F("media"));
	media[F("contentId")] = url;
	media[F("contentType")] = mime;
	media[F("streamType")] = F("BUFFERED");

	return send(doc);
}

bool Media::pause(int sessionId)
{
	StaticJsonDocument<200> doc;
	initRequest(doc, F("PAUSE"));
	doc[F("mediaSessionId")] = sessionId;

	return send(doc);
}

bool Media::play(int sessionId)
{
	StaticJsonDocument<200> doc;
	initRequest(doc, F("PLAY"));
	doc[F("mediaSessionId")] = sessionId;

	return send(doc);
}

bool Media::stop(int sessionId)
{
	StaticJsonDocument<200> doc;
	initRequest(doc, F("STOP"));
	doc[F("mediaSessionId")] = sessionId;

	return send(doc);
}

bool Media::seek(int sessionId, float currentTime, State newState)
{
	StaticJsonDocument<200> doc;
	initRequest(doc, F("SEEK"));
	doc[F("mediaSessionId")] = sessionId;
	if(newState != State::NoChange) {
		doc[F("resumeState")] = (newState == State::Pause) ? F("PLAYBACK_PAUSE") : F("PLAYBACK_START");
	}
	doc[F("currentTime")] = currentTime;

	return send(doc);
}

} // namespace GoogleCast
