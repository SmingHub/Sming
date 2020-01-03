/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Session.h
 *
 ****/

#pragma once

#include "Context.h"
#include "KeyCertPair.h"
#include "ValidatorList.h"
#include <Platform/System.h>

class TcpConnection;

namespace Ssl
{
/**
 * @brief Indicate to SSL how much memory (approximately) to commit for buffers
 *
 * A remote SSL server may require data transfers in large (16K) fragments,
 * so restricting buffer sizes may cause connections to such servers to fail.
 *
 * This must be balanced against other requirements for RAM by the application,
 * therefore this setting can be used to restrict RAM usage.
 *
 * @note The ordinal value of this enumeration corresponds to SSL fragment size as defined in
 * Maximum Fragment Length Negotiation https://tools.ietf.org/html/rfc6066
 */
enum class MaxBufferSize {
	Default = 0, ///< Let SSL implementation decide
	B512,		 ///< 512 bytes
	K1,			 ///< 1024 bytes
	K2,
	K4,
	K8,
	K16,
};

__forceinline size_t maxBufferSizeToBytes(MaxBufferSize value)
{
	return (value == MaxBufferSize::Default) ? 0 : 256U << size_t(value);
}

/**
 * @brief Configurable options
 */
struct Options {
	bool sessionResume : 1; ///< Keep a note of session ID for later re-use
	bool clientAuthentication : 1;
	bool verifyLater : 1; ///< Allow handshake to complete before verifying certificate
	bool freeKeyCertAfterHandshake : 1;

	Options() : sessionResume(false), clientAuthentication(false), verifyLater(false), freeKeyCertAfterHandshake(false)
	{
	}

	String toString() const;
};

/**
 * @brief Handles all SSL activity for a TCP connection
 *
 * Each TCP connection creates a session at a suitable
 * ManagerTop-level layer for SSL Contains configuration for each SSL connection
 */
class Session
{
public:
	using InitDelegate = Delegate<void(Session& session)>;

	String hostName; ///< Used for SNI https://en.wikipedia.org/wiki/Server_Name_Indication
	KeyCertPair keyCert;
	Options options;
	MaxBufferSize maxBufferSize = MaxBufferSize::Default;
	/**
	 * Server: Number of cached client sessions. Suggested value: 10
	 * Client: Number of cached session ids. Suggested value: 1
	 */
	int cacheSize = 10;
	// client
	ValidatorList validators;

public:
	~Session()
	{
		close();
		delete sessionId;
	}

	const SessionId* getSessionId() const
	{
		return sessionId;
	}

	/**
	 * @brief Called when a client connection is made via server TCP socket
	 * @brief client The client TCP socket
	 * @brief tcp The low-level TCP connection to use for reading and writing
	 */
	bool onAccept(TcpConnection* client, tcp_pcb* tcp);

	/**
	 * @brief Called by TcpConnection to set the established SSL connection
	 */
	void setConnection(Connection* connection)
	{
		assert(this->connection == nullptr);
		this->connection = connection;
	}

	/**
	 * @brief Get the currently active SSL connection object
	 */
	Connection* getConnection()
	{
		return connection;
	}

	/**
	 * @brief Handle connection event
	 * @retval err_t
	 */
	bool onConnect(tcp_pcb* tcp);

	/**
	 * @brief Determine if an SSL connection has been fully established
	 */
	bool isConnected() const
	{
		return connection ? connection->isHandshakeDone() : false;
	}

	/**
	 * @brief End the session
	 */
	void close();

	/**
	 * @brief Read data from SSL connection
	 * @param input Source encrypted data
	 * @param output Points to decrypted content
	 * @retval int Size of decrypted data returned, or negative on error
	 */
	int read(InputBuffer& input, uint8_t*& output);

	/**
	 * @brief Write data to SSL connection
	 * @param data
	 * @param length
	 * @retval int Quantity of bytes actually written, or tcp error code
	 */
	int write(const uint8_t* data, size_t length);

	/**
	 * @brief Called by SSL adapter when certificate validation is required
	 * @retval bool true if validation is success, false to abort connection
	 */
	bool validateCertificate();

	/**
	 * @brief Called by SSL adapter when handshake has been completed
	 * @param success Indicates if handshake was successful
	 */
	void handshakeComplete(bool success);

	/**
	 * @brief For debugging
	 */
	size_t printTo(Print& p) const;

private:
	void beginHandshake();
	void endHandshake();

private:
	Context* context = nullptr;
	Connection* connection = nullptr;
	SessionId* sessionId = nullptr;
	CpuFrequency curFreq = CpuFrequency(0);
};

}; // namespace Ssl
