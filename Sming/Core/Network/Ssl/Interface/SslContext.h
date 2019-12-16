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

#include <user_config.h>
#include "../SslStructs.h"
#include "SslExtension.h"

class SslConnection;

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

/**
 * @brief Describes the different types of data that can be added to a context
 */
enum SslContextObject {
	eSCO_X509_CERT = 1, // << certificate - can be client or server one
	eSCO_X509_CACERT,   // << Certificate Authority certificate
	eSCO_RSA_KEY,		// << RSA key - can be public or private
	eSCO_PKCS8,
	eSCO_PKCS12
};

class SslContext
{
public:
	virtual ~SslContext()
	{
	}

	/**
	 * @brief Initializer method that must be called after object creation and before the creation
	 * 		  of server or client connections
	 * @param tcp active tcp connection
	 * @param options
	 * @param sessionCacheSize
	 *
	 * @retval bool true on success
	 */
	virtual bool init(tcp_pcb* tcp, uint32_t options = 0, size_t sessionCacheSize = 1)
	{
		this->tcp = tcp;
		this->options = options;
		this->sessionCacheSize = sessionCacheSize;
		return true;
	}

	/**
	 * @brief Use to load into memory certificates, public and private keys, etc.
	 * @param memType the type of the data. Example: public key, client certificate, etc
	 * @param data - the data should be in DER format ( https://wiki.openssl.org/index.php/DER )
	 * @param length
	 * @param password - null terminated string
	 *
	 * @retval boo true on success
	 */
	virtual bool loadMemory(const SslContextObject& memType, const uint8_t* data, size_t length,
							const char* password) = 0;

	/**
	 * @brief Creates client SSL connection.
	 *        Your SSL client use this call to make create a client connection to remote server.
	 * @param sessionId* - if provided will try to use the sessionId for SSL resumption.
	 * 					  This will speed up consecutive SSL handshakes to the same server on the same port
	 * @param sslExtensions*
	 *
	 * @retval SslConnection*
	 */
	SslConnection* createClient(SslSessionId* sessionId, SslExtension* sslExtensions)
	{
		return doCreateClient(sessionId != nullptr ? sessionId->getValue() : nullptr,
							  sessionId != nullptr ? sessionId->getLength() : 0, sslExtensions);
	}

	/**
	 * @brief Creates server SSL connection.
	 *        Your SSL servers use this call to allow remote clients to connect to them and use SSL.
	 * @retval SslConnection*
	 */
	virtual SslConnection* createServer() = 0;

	/**
	 * @brief Returns the current active tcp connection that is used
	 * @retval tcp_pcb
	 */
	tcp_pcb& getTcp()
	{
		return *tcp;
	}

protected:
	/**
	 * @brief Creates client SSL connection.
	 *        Each SSL implementations should implement this method
	 * @param sessionData
	 * @param length session data length
	 * @param SslExtension*
	 *
	 * @retval SslConnection*
	 */
	virtual SslConnection* doCreateClient(const uint8_t* sessionData, size_t length, SslExtension* sslExtensions) = 0;

protected:
	tcp_pcb* tcp = nullptr;  // << should contain active tcp connection
	uint32_t options;		 // << SSL context options
	size_t sessionCacheSize; // << Session Cache Size
		//    Server: When the context is used to create a server connection this indicates how many
		//    		client sessions will be cached. Suggested value: 10
		//    Client: When used to create a client connection this indicates how many ssl sessio ids
		//             should be cached . Suggested value: 1
};

/** @} */
