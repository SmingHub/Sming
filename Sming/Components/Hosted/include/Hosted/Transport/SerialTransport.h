/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SerialTransport.h
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#pragma once

#include <HardwareSerial.h>
#include "BaseTransport.h"

namespace Hosted::Transport
{
class SerialTransport : public BaseTransport
{
public:
	SerialTransport(HardwareSerial& stream)
	{
		stream.onDataReceived(StreamDataReceivedDelegate(&SerialTransport::process, this));
	}

private:
	void process(Stream& source, char arrivedChar, uint16_t availableCharsCount)
	{
		handler(source);
	}
};

} // namespace Hosted::Transport
