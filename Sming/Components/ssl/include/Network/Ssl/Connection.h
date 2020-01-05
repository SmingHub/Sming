/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Connection.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "SessionId.h"
#include "Certificate.h"
#include "InputBuffer.h"
#include "CipherSuite.h"
#include "Alert.h"

namespace Ssl
{
class Context;

/**
 * @brief Implemented by SSL adapter to handle a connection
 *
 * Returned `int` error codes are 0 for success, or < 0 for error.
 *
 * The error codes themselves are implementation-specific.
 * Use `getErrorString()` to obtain the message.
 * SSL Alerts are also reported via error codes and can be obtained
 * using a call to `getAlert()`.
 */
class Connection : public Printable
{
public:
	Connection(Context& context, tcp_pcb* tcp) : context(context), tcp(tcp)
	{
		assert(tcp != nullptr);
	}

	virtual ~Connection()
	{
	}

	/**
	 * @brief Checks if the handshake has finished
	 * @retval bool true on success
	 */
	virtual bool isHandshakeDone() const = 0;

	/**
	 * @brief Reads encrypted information and decrypts it
	 * @param input Source encrypted data
	 * @param output Pointer to decrypted plaintext buffer
	 *
	 * @retval
	 * 		   0 : handshake is still in progress
	 * 		 > 0 : there is decrypted data
	 * 		 < 0 : error
	 */
	virtual int read(InputBuffer& input, uint8_t*& output) = 0;

	/**
	 * @brief Converts and sends plaintext data
	 * @param data
	 * @param length
	 * @retval int length of the data that was actually written
	 * 	< 0 on error
	 */
	virtual int write(const uint8_t* data, size_t length) = 0;

	/**
	 * @brief Gets the cipher suite that was used
	 * @retval CipherSuite IDs as defined by SSL/TLS standard
	 */
	virtual CipherSuite getCipherSuite() const = 0;

	/**
	 * @brief Gets the current session id object.
	 *        Should be called after handshake.
	 * @retval SessionId
	 */
	virtual SessionId getSessionId() const = 0;

	/**
	 * @brief Gets the certificate object.
	 *        That object MUST be owned by the Connection implementation
	 *        and should not be freed outside of it
	 *
	 * @retval Certificate* Returns NULL if there is no certificate available
	 */
	virtual const Certificate* getCertificate() const = 0;

	virtual void freeCertificate() = 0;

	/**
	 * @brief For debugging
	 */
	size_t printTo(Print& p) const override;

	int writeTcpData(uint8_t* data, size_t length);

	/**
	 * @brief Get string for error code
	 */
	virtual String getErrorString(int error) const = 0;

	/**
	 * @brief Get alert code from error
	 * @param error
	 * @retval Alert Alert::INVALID if not an alert
	 */
	virtual Alert getAlert(int error) const = 0;

protected:
	Context& context;
	tcp_pcb* tcp;
};

} // namespace Ssl
