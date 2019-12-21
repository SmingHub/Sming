/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ContextImpl.cpp
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "ContextImpl.h"
#include "ConnectionImpl.h"

namespace Ssl
{
ContextImpl::~ContextImpl()
{
	// Free context typically sends out closing message
	ssl_ctx_free(context);
	// Now we can free the connection
	axl_free(tcp);
}

bool ContextImpl::init(tcp_pcb* tcp, uint32_t options, size_t sessionCacheSize)
{
	if(!Context::init(tcp, options, sessionCacheSize)) {
		return false;
	}

	axl_init(capacity);
	ssl_ctx_free(context);
	context = ssl_ctx_new(SSL_CONNECT_IN_PARTS | options, sessionCacheSize);

	return (context != nullptr);
}

Ssl::Connection* ContextImpl::internalCreateClient(const uint8_t* sessionData, size_t sessionLength,
												   Extension* extension)
{
	int clientfd = axl_append(tcp);
	if(clientfd < 0) {
		debug_d("SSL: Unable to add LWIP tcp -> clientfd mapping");
		return nullptr;
	}

	SSL_EXTENSIONS* sslExtensions = nullptr;
	if(extension != nullptr) {
		sslExtensions = ssl_ext_new();
		ssl_ext_set_host_name(sslExtensions, extension->hostName.c_str());
		ssl_ext_set_max_fragment_size(sslExtensions, extension->fragmentSize);
	}

	SSL* ssl = ssl_client_new(context, clientfd, sessionData, sessionLength, sslExtensions);
	if(ssl == nullptr) {
		delete sslExtensions;
		return nullptr;
	}

	return new ConnectionImpl(ssl);
}

Ssl::Connection* ContextImpl::createServer()
{
	int clientfd = axl_append(tcp);
	if(clientfd < 0) {
		debug_d("SSL: Unable to add LWIP tcp -> clientfd mapping");
		return nullptr;
	}

	SSL* ssl = ssl_server_new(context, clientfd);
	if(ssl == nullptr) {
		return nullptr;
	}

	return new ConnectionImpl(ssl);
}

bool ContextImpl::loadMemory(ObjectType memType, const uint8_t* data, size_t length, const char* password)
{
	return (ssl_obj_memory_load(context, int(memType), data, length, password) == SSL_OK);
}

} // namespace Ssl
