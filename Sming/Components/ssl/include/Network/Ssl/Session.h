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
#include "Validator.h"
#include <Platform/System.h>

class TcpConnection;

namespace Ssl
{
/**
 * @brief Maximum Fragment Length Negotiation https://tools.ietf.org/html/rfc6066
 *
 * 0,1,2,3..6 corresponding to off,512,1024,2048..16384 bytes
 *
 * The allowed values for this field are: 2^9, 2^10, 2^11, and 2^12
 *
 */
enum FragmentSize {
	eSEFS_Off, ///< Let SSL implementation decide
	eSEFS_512, ///< 512 bytes
	eSEFS_1K,  ///< 1024 bytes
	eSEFS_2K,
	eSEFS_4K,
	eSEFS_8K,
	eSEFS_16K,
};

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

class Session
{
public:
	using InitDelegate = Delegate<void(Session& session)>;

	String hostName;
	KeyCertPair keyCert;
	Options options;
	FragmentSize fragmentSize = eSEFS_Off;
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

	bool onAccept(TcpConnection* client, tcp_pcb* tcp);

	void setConnection(Connection* connection)
	{
		assert(this->connection == nullptr);
		this->connection = connection;
	}

	Connection* getConnection()
	{
		return connection;
	}

	/**
	 * @brief Handle connection event
	 * @retval err_t
	 */
	bool onConnect(tcp_pcb* tcp);

	bool isConnected() const
	{
		return connection ? connection->isHandshakeDone() : false;
	}

	/**
	 * @brief End the session
	 */
	void close();

	int read(InputBuffer& input, uint8_t*& output);

	/**
	 * @brief Write data to SSL connection
	 * @param data
	 * @param length
	 * @retval int Quantity of bytes actually written, or tcp error code
	 */
	int write(const uint8_t* data, size_t length);

	bool validateCertificate();
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
