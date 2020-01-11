/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * X509Context.cpp
 *
 ****/

#include "X509Context.h"

namespace Ssl
{
const br_x509_class X509Context::x509_class PROGMEM = {
	sizeof(X509Context), start_chain, start_cert, append, end_cert, end_chain, get_pkey,
};

} // namespace Ssl
