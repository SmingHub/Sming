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

/**
 * @ingroup ssl
 * @brief Provides the functionality needed to create SSL communicate over TCP
 * @{
 */

/* errors that can be generated */
#define SSL_OK 0
#define SSL_NOT_OK -1
#define SSL_ERROR_DEAD -2
#define SSL_CLOSE_NOTIFY -3
#define SSL_ERROR_CONN_LOST -256
#define SSL_ERROR_RECORD_OVERFLOW -257
#define SSL_ERROR_SOCK_SETUP_FAILURE -258
#define SSL_ERROR_INVALID_HANDSHAKE -260
#define SSL_ERROR_INVALID_PROT_MSG -261
#define SSL_ERROR_INVALID_HMAC -262
#define SSL_ERROR_INVALID_VERSION -263
#define SSL_ERROR_UNSUPPORTED_EXTENSION -264
#define SSL_ERROR_INVALID_SESSION -265
#define SSL_ERROR_NO_CIPHER -266
#define SSL_ERROR_INVALID_CERT_HASH_ALG -267
#define SSL_ERROR_BAD_CERTIFICATE -268
#define SSL_ERROR_INVALID_KEY -269
#define SSL_ERROR_FINISHED_INVALID -271
#define SSL_ERROR_NO_CERT_DEFINED -272
#define SSL_ERROR_NO_CLIENT_RENOG -273
#define SSL_ERROR_NOT_SUPPORTED -274
#define SSL_X509_OFFSET -512

/* alert types that are recognized */
#define SSL_ALERT_TYPE_WARNING 1
#define SLL_ALERT_TYPE_FATAL 2

/* these are all the alerts that are recognized */
#define SSL_ALERT_CLOSE_NOTIFY 0
#define SSL_ALERT_UNEXPECTED_MESSAGE 10
#define SSL_ALERT_BAD_RECORD_MAC 20
#define SSL_ALERT_RECORD_OVERFLOW 22
#define SSL_ALERT_HANDSHAKE_FAILURE 40
#define SSL_ALERT_BAD_CERTIFICATE 42
#define SSL_ALERT_UNSUPPORTED_CERTIFICATE 43
#define SSL_ALERT_CERTIFICATE_EXPIRED 45
#define SSL_ALERT_CERTIFICATE_UNKNOWN 46
#define SSL_ALERT_ILLEGAL_PARAMETER 47
#define SSL_ALERT_UNKNOWN_CA 48
#define SSL_ALERT_DECODE_ERROR 50
#define SSL_ALERT_DECRYPT_ERROR 51
#define SSL_ALERT_INVALID_VERSION 70
#define SSL_ALERT_NO_RENEGOTIATION 100
#define SSL_ALERT_UNSUPPORTED_EXTENSION 110

class SslConnection
{
public:
	virtual ~SslConnection()
	{
	}

	/**
	 * @brief Checks if the handshake has finished
	 * @retval bool true on success
	 */
	virtual bool isHandshakeDone() = 0;

	/**
	 * @brief Reads encrypted information and decrypts it
	 * @param tcp active tcp connection
	 * @param encrypted - the pbuf buffer will the encrypted data
	 * @param
	 *
	 * @retval
	 * 		 0 - when the handshake is still in progress
	 * 		 > 0 - when the is decrypted data
	 * 		 < 0 - in case of an error
	 */
	virtual int read(tcp_pcb* tcp, pbuf* encrypted, pbuf** decrypted) = 0;

	/**
	 * @brief Converts and sends plaintext data
	 * @param data
	 * @param length
	 * @retval int length of the data that was actually written
	 */
	virtual int write(const uint8_t* data, size_t length) = 0;

	/**
	 * @brief Calculates the size of encrypted plain text data
	 * @param plainTextLength
	 * @retval int
	 */
	virtual int calcWriteSize(size_t plainTextLength) = 0;

	/**
	 * @brief Gets the name of the cipher that was used
	 * @retval String
	 */
	virtual const String getCipher() const = 0;

	/**
	 * @brief Gets the current session id object.
	 *        Should be called after handshake.
	 * @retval SslSessionId*
	 */
	virtual SslSessionId* getSessionId() = 0;

	/**
	 * @brief Gets the certificate object.
	 *        That object MUST be owned by the SslConnection implementation
	 *        and should not be freed outside of it
	 *
	 * @retval SslSessionId*
	 */
	virtual SslCertificate* getCertificate() = 0;
};

/** @} */
