/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_NETWORK_SSL_SSLSTRUCTS_H_
#define SMINGCORE_NETWORK_SSL_SSLSTRUCTS_H_

#include "SslFingerprints.h"
#include "SslKeyCertPair.h"
#include "SslSessionId.h"

/*
 * These structures have been renamed, please use the revised convention SslXXX
 */
typedef SslKeyCertPair SSLKeyCertPair __deprecated;
typedef SslSessionId SSLSessionId __deprecated;
typedef SslFingerprints SSLFingerprints __deprecated;

#endif /* SMINGCORE_NETWORK_SSL_SSLSTRUCTS_H_ */
