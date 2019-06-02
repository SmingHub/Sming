/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpServer.h
 *
 ****/

/** @defgroup   ftpserver FTP server
 *  @brief      Provides FTP server
 *  @ingroup    tcpserver
 *  @{
 */

#pragma once

#include "TcpServer.h"
#include "WHashMap.h"
#include "WString.h"

class FtpServerConnection;

class FtpServer : public TcpServer
{
	friend class FtpServerConnection;

public:
	FtpServer()
	{
		setTimeOut(900); // Update timeout
	}

	void addUser(const String& login, const String& pass);
	bool checkUser(const String& login, const String& pass);

protected:
	TcpConnection* createClient(tcp_pcb* clientTcp) override;

	virtual bool onCommand(String cmd, String data, FtpServerConnection& connection);

private:
	HashMap<String, String> users;
};

typedef FtpServer FTPServer SMING_DEPRECATED; ///< @deprecated Use `FtpServer` instead

/** @} */
