/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BaseTransport.h
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#pragma once

#include <Stream.h>
#include <Delegate.h>

namespace Hosted
{
namespace Transport
{
class BaseTransport
{
public:
	using DataHandler = Delegate<bool(Stream&)>;

	void onData(DataHandler handler)
	{
		this->handler = handler;
	}

	virtual ~BaseTransport()
	{
	}

protected:
	DataHandler handler;
};

} // namespace Transport

} // namespace Hosted
