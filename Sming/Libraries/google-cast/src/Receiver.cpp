/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Client.cpp
 *
 ****/

#include "Network/GoogleCast/Receiver.h"

namespace GoogleCast
{
bool Receiver::getStatus()
{
	StaticJsonDocument<200> req;
	initRequest(req, F("GET_STATUS"));
	return send(req);
}

bool Receiver::launch(const String& appId)
{
	StaticJsonDocument<200> doc;
	initRequest(doc, F("LAUNCH"));
	doc[F("appId")] = appId;

	return send(doc);
}

bool Receiver::isAppAvailable(Vector<String> appIds)
{
	// TODO
	return false;
}

bool Receiver::setVolumeLevel(float level)
{
	StaticJsonDocument<200> doc;
	initRequest(doc, F("SET_VOLUME"));
	doc[F("level")] = level;

	return send(doc);
}

bool Receiver::setVolumeMuted(bool muted)
{
	StaticJsonDocument<200> doc;
	initRequest(doc, F("SET_VOLUME"));
	doc[F("volume")] = muted;

	return send(doc);
}

} // namespace GoogleCast