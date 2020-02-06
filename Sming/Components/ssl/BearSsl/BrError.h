/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrError.h
 *
 ****/

#pragma once

#include <WString.h>

namespace Ssl
{
String getErrorString(int error);
Alert getAlert(int error);
} // namespace Ssl
