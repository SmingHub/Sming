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

#include "Connection.h"
#include "KeyCertPair.h"
#include "Validator.h"

struct tcp_pcb;

namespace Ssl
{
class Session;

/**
 * @brief Implemented by SSL adapter to create and manage SSL connections
 */
class Context
{
public:
	Context(Session& session) : session(session)
	{
	}

	virtual ~Context()
	{
	}

	/**
	 * @brief Initializer method that must be called after object creation and before the creation
	 * 		  of server or client connections
	 * @retval bool true on success
	 */
	virtual bool init() = 0;

	/**
	 * @brief Creates client SSL connection.
	 *        Your SSL client use this call to create a client connection to remote server.
	 * @retval Connection*
	 */
	virtual Connection* createClient(tcp_pcb* tcp) = 0;

	/**
	 * @brief Creates server SSL connection.
	 *        Your SSL servers use this call to allow remote clients to connect to them and use SSL.
	 * @retval Connection*
	 */
	virtual Connection* createServer(tcp_pcb* tcp) = 0;

public:
	Session& session;
};

} // namespace Ssl
