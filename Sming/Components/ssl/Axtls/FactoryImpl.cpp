/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FactoryImpl.cpp
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include <Network/Ssl/Factory.h>
#include "ContextImpl.h"

namespace Ssl
{
class AxtlsFactory : public Factory
{
public:
	Context* createContext() override
	{
		return new Ssl::ContextImpl();
	}
};

static AxtlsFactory axtlsFactory;
Factory* factory = &axtlsFactory;

} // namespace Ssl
