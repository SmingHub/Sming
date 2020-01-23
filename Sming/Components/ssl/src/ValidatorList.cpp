/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ValidatorList.cpp
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include <Network/Ssl/ValidatorList.h>
#include <debug_progmem.h>

namespace Ssl
{
bool ValidatorList::validate(const Certificate* certificate)
{
	if(certificate == nullptr) {
		removeAllElements();
		return false;
	}

	if(isEmpty()) {
		debug_w("SSL Validator: list empty, allow connection");
		return true;
	}

	bool success = false;
	while(!isEmpty()) {
		if(!success) {
			success = operator[](0).validate(*certificate);
		}
		remove(0);
	}

	debug_w("SSL validator: %s", success ? _F("Success") : _F("NO match"));

	return success;
}

} // namespace Ssl
