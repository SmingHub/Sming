/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Connection.h
 *
 ****/

#pragma once

#include "Channel.h"

namespace GoogleCast
{
class Connection : public Channel
{
public:
	using Channel::Channel;

	String getNameSpace() const override
	{
		return F("urn:x-cast:com.google.cast.tp.connection");
	}

	bool connect()
	{
		return sendSimpleMessage(F("CONNECT"));
	}

	bool close()
	{
		return sendSimpleMessage(F("CLOSE"));
	}
};

} // namespace GoogleCast
