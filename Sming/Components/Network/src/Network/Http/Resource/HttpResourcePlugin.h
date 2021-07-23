/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 *
 * @author: 2021 - Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#pragma once

#include "HttpEventedResource.h"
#include <Data/LinkedObjectList.h>

class HttpResourcePlugin : public LinkedObjectTemplate<HttpResourcePlugin>
{
public:
	using OwnedList = OwnedLinkedObjectListTemplate<HttpResourcePlugin>;

	virtual bool registerPlugin(HttpEventedResource& resource) = 0;
};
