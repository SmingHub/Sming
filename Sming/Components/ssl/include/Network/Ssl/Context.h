/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Context.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "Extension.h"
#include "Connection.h"
#include "SessionId.h"

struct tcp_pcb;

namespace Ssl
{
/**
 * @ingroup ssl
 * @brief Encapsulates operations related to creating a SSL context.
 * 		  The SSL context is the one that can later on be used to create client or server SSL connections
 * @{
 */

// SSL Options
#define SSL_CLIENT_AUTHENTICATION 0x00010000
#define SSL_SERVER_VERIFY_LATER 0x00020000
#define SSL_NO_DEFAULT_KEY 0x00040000
#define SSL_DISPLAY_STATES 0x00080000
#define SSL_DISPLAY_BYTES 0x00100000
#define SSL_DISPLAY_CERTS 0x00200000
#define SSL_DISPLAY_RSA 0x00400000

class Context
{
public:
	/**
	 * @brief Describes the different types of data that can be added to a context
	 * @note These are defined by AXTLS. Other implementations will need to translate them.
	 */
	enum class ObjectType {
		X509_CERT = 1, // << certificate - can be client or server one
		X509_CACERT,   // << Certificate Authority certificate
		RSA_KEY,	   // << RSA key - can be public or private
		PKCS8,
		PKCS12,
	};

	virtual ~Context()
	{
	}

	/**
	 * @brief Initializer method that must be called after object creation and before the creation
	 * 		  of server or client connections
	 * @param tcp active tcp connection
	 *
	 * @param options
	 *
	 * @param sessionCacheSize
	 * 		Server: When the context is used to create a server connection this indicates how many
	 * 			client sessions will be cached. Suggested value: 10
	 * 		Client: When used to create a client connection this indicates how many ssl sessio ids
	 * 			should be cached . Suggested value: 1
	 *
	 * @retval bool true on success
	 */
	virtual bool init(tcp_pcb* tcp, uint32_t options = 0, size_t sessionCacheSize = 1) = 0;

	/**
	 * @brief Use to load into memory certificates, public and private keys, etc.
	 * @param memType the type of the data. Example: public key, client certificate, etc
	 * @param data The data should be in DER format ( https://wiki.openssl.org/index.php/DER )
	 * @param length Size of data in bytes
	 * @param password - null terminated string
	 *
	 * @retval boo true on success
	 */
	virtual bool loadMemory(ObjectType memType, const uint8_t* data, size_t length, const char* password) = 0;

	/**
	 * @brief Creates client SSL connection.
	 *        Your SSL client use this call to make create a client connection to remote server.
	 * @param sessionId   If provided, will try to use the sessionId for SSL resumption.
	 * 					  This will speed up consecutive SSL handshakes to the same server on the same port
	 * @param extension   Additional details required for connection
	 *
	 * @retval Connection*
	 */
	virtual Connection* createClient(SessionId* sessionId, const Extension& extension) = 0;

	/**
	 * @brief Creates server SSL connection.
	 *        Your SSL servers use this call to allow remote clients to connect to them and use SSL.
	 * @retval Connection*
	 */
	virtual Connection* createServer() = 0;
};

/** @} */

} // namespace Ssl
