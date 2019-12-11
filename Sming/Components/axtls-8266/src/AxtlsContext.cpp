/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxtlsContext.cpp
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "AxtlsContext.h"
#include "AxtlsConnection.h"

bool AxtlsContext::init()
{
	axl_init(fdCount);
	ssl_ctx_free(context);
	context = ssl_ctx_new(SSL_CONNECT_IN_PARTS | options, sessionCacheSize);

	return (context!= nullptr);
}

SslConnection* AxtlsContext::doCreateClient(const uint8_t* sessionData, size_t sessionLength, SslExtension* sslExtension)
{
	int clientfd = axl_append(tcp);
	if(clientfd < 0) {
		debug_d("SSL: Unable to add LWIP tcp -> clientfd mapping");
		return nullptr;
	}

	SSL_EXTENSIONS* extensions = reinterpret_cast<SSL_EXTENSIONS*>(sslExtension->getInternalObject());
	SSL* ssl = ssl_client_new(context, clientfd, sessionData, sessionLength, extensions);
	if(ssl == nullptr) {
		return nullptr;
	}

	return new AxtlsConnection(ssl);
}

SslConnection* AxtlsContext::createServer()
{
	int clientfd = axl_append(tcp);
	if (clientfd < 0) {
		debug_d("SSL: Unable to add LWIP tcp -> clientfd mapping");
		return nullptr;
	}

	SSL* ssl = ssl_server_new(context, clientfd);
	if(ssl == nullptr) {
		return nullptr;
	}

	return new AxtlsConnection(ssl);
}

bool AxtlsContext::loadMemory(int memType, const uint8_t *data, size_t length, const char *password)
{
	return (ssl_obj_memory_load(context, memType, data, length, password) != SSL_OK);
}

AxtlsContext::~AxtlsContext()
{
	axl_free(tcp);
	ssl_ctx_free(context);
}
