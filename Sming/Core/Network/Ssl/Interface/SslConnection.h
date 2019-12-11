/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslInterface.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/
#pragma once

#include "../SslStructs.h"
#include "SslCertificate.h"

class SslConnection {
public:
	virtual bool isHandshakeDone() = 0;
	virtual int read(tcp_pcb* tcp, pbuf* encrypted, pbuf** decrypted) = 0;
	virtual int write(const uint8_t* data, size_t length) = 0;
	virtual int calcWriteSize(size_t plainTextLength) = 0;
	virtual const String getCipher() const = 0;
	virtual SslSessionId* getSessionId() = 0;
	virtual SslCertificate* getCertificate() = 0;

	virtual ~SslConnection() {
	}
};
