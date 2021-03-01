/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Receiver.h
 *
 ****/

#pragma once

#include "Channel.h"

namespace GoogleCast
{
class Receiver : public Channel
{
public:
	using Channel::Channel;

	String getNameSpace() const override
	{
		return F("urn:x-cast:com.google.cast.receiver");
	}

	bool launch(const String& appId);

	bool getStatus()
	{
		return sendMessage(F("GET_STATUS"), true);
	}

	bool isAppAvailable(Vector<String> appIds);

	/**
	 * @brief: Sets volume.
	 * @param level is a float between 0 and 1
	 *
	 * @return bool
	 */
	bool setVolumeLevel(float level);

	bool setVolumeMuted(bool muted);
};

} // namespace GoogleCast