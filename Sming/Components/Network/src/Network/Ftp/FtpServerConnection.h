/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpServerConnection.h
 *
 ****/

#pragma once

#include "Network/TcpConnection.h"
#include <IpAddress.h>
#include "WString.h"
#include <IFS/FileSystem.h>

/**
 * @defgroup ftp FTP
 * @ingroup  tcp
 * @{
 */

class CustomFtpServer;
class FtpDataStream;

class FtpServerConnection : public TcpConnection
{
	friend class FtpDataStream;
	friend class CustomFtpServer;

public:
	struct User {
		CString name;
		IFS::UserRole role{};

		bool isValid() const
		{
			return role != IFS::UserRole::None;
		}
	};

	static constexpr size_t MAX_FTP_CMD{255};

	FtpServerConnection(CustomFtpServer& parentServer, tcp_pcb* clientTcp);

	err_t onReceive(pbuf* buf) override;
	err_t onSent(uint16_t len) override;

	void dataTransferFinished(TcpConnection* connection);
	void dataStreamDestroyed(TcpConnection* connection);

	const User& getUser() const
	{
		return user;
	}

	// Get the active filesystem, send an error response if undefined
	IFS::FileSystem* getFileSystem();

	virtual void response(int code, String text = nullptr, char sep = ' ');

protected:
	virtual void onCommand(String cmd, String data);

	void cmdPort(const String& data);
	void setDataConnection(FtpDataStream* connection);
	String resolvePath(const char* name);
	bool checkFileAccess(const char* filename, IFS::OpenFlags flags);

private:
	CustomFtpServer& server;
	User user{};
	CString renameFrom;

	IpAddress ip;
	uint16_t port{20};
	bool readyForData{false};
	CString cwd;
	FtpDataStream* dataConnection{nullptr};
};

/** @} */
