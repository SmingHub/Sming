/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslImplementation.h
 *
 * SSL implementation based on axTLS library
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "AxtlsCertificate.h"
#include "AxtlsContext.h"
#include "AxtlsConnection.h"
#include "AxtlsExtension.h"

typedef AxtlsContext SslContextImpl;
typedef AxtlsConnection SslConnectionImpl;
typedef AxtlsExtension SslExtensionImpl;
typedef AxtlsCertificate SslCertImplementation;
