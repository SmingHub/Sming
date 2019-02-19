/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
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

#ifndef _SMING_CORE_NETWORK_FTP_SERVER_H_
#define _SMING_CORE_NETWORK_FTP_SERVER_H_

#include "TcpServer.h"
#include "WHashMap.h"
#include "WVector.h"
#include "WString.h"

class FtpServerConnection;

class FtpServer : public TcpServer
{
	friend class FtpServerConnection;

public:
	FtpServer();
	~FtpServer();

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
#endif /* _SMING_CORE_NETWORK_FTP_SERVER_H_ */
