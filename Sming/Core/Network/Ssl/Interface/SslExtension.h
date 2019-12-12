/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslInterface.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/
#pragma once

#include <WString.h>

class SslExtension
{
public:
	virtual bool setHostName(const String& hostName) = 0;

	// TODO: Fragment size should be constant ....
	virtual bool setMaxFragmentSize(uint8_t fragmentSize) = 0;

	virtual void* getInternalObject() = 0;

	virtual ~SslExtension()
	{
	}
};
