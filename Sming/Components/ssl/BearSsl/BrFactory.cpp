/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FactoryImpl.cpp
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#include <Network/Ssl/Factory.h>
#include "BrContext.h"

namespace Ssl
{
class BrFactory : public Factory
{
public:
	Context* createContext(Session& session) override
	{
		return new BrContext(session);
	}
};

static BrFactory brFactory;
Factory* factory = &brFactory;

} // namespace Ssl
