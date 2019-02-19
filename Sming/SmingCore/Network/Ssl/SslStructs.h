/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslStructs.h
 *
 ****/

#ifndef _SMING_CORE_NETWORK_SSL_SSL_STRUCTS_H_
#define _SMING_CORE_NETWORK_SSL_SSL_STRUCTS_H_

#include "SslFingerprints.h"
#include "SslKeyCertPair.h"
#include "SslSessionId.h"

/*
 * These structures have been renamed, please use the revised convention SslXXX
 */
typedef SslKeyCertPair SSLKeyCertPair SMING_DEPRECATED;			///< @deprecated Use SslKeyCertPair instead
typedef SslSessionId SSLSessionId SMING_DEPRECATED;				///< @deprecated Use SslSessionId instead
typedef SslFingerprints SSLFingerprints SMING_DEPRECATED;		///< @deprecated Use SslFingerprints instead
typedef SslFingerprintType SSLFingerprintType SMING_DEPRECATED; ///< @deprecated Use SslFingerprintType instead

#endif /* _SMING_CORE_NETWORK_SSL_SSL_STRUCTS_H_ */
