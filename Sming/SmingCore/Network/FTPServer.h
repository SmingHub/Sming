/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/** @defgroup   ftpserver FTP server
 *  @brief      Provides FTP server
 *  @ingroup    tcpserver
 *  @{
 */

#ifndef _SMING_CORE_FTPSERVER_H_
#define _SMING_CORE_FTPSERVER_H_

#include "TcpServer.h"
#include "WHashMap.h"
#include "WVector.h"
#include "WString.h"

class FTPServerConnection;

class FTPServer : public TcpServer
{
	friend class FTPServerConnection;

public:
	FTPServer();
	~FTPServer();

	void addUser(const String& login, const String& pass);
	bool checkUser(const String& login, const String& pass);

protected:
	TcpConnection* createClient(tcp_pcb* clientTcp) override;

	virtual bool onCommand(String cmd, String data, FTPServerConnection& connection);

private:
	HashMap<String, String> users;
};

/** @} */
#endif /* _SMING_CORE_FTPServer_H_ */
