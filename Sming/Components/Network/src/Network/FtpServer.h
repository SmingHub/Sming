/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpServer.h
 *
 ****/

#pragma once

#include "TcpServer.h"
#include "WHashMap.h"
#include <FileSystem.h>

class FtpServerConnection;

/** @defgroup   ftpserver FTP server
 *  @ingroup    tcpserver
 *  @brief      Base implementation for FTP server
 */
class CustomFtpServer : public TcpServer
{
	friend class FtpServerConnection;

public:
	CustomFtpServer(IFS::FileSystem* fileSystem = nullptr) : fileSystem(fileSystem)
	{
		setTimeOut(900);
	}

	/**
	 * @brief Validate user
	 * @param login User name
	 * @param pass User password
	 * @retval IFS::UserRole Returns assigned user role, None if user not validated
	 */
	virtual IFS::UserRole validateUser(const char* login, const char* pass) = 0;

protected:
	TcpConnection* createClient(tcp_pcb* clientTcp) override;

	/**
	 * @brief Handle an incomding command
	 * @param cmd The command identifier, e.g. LIST
	 * @param data Any command arguments
	 * @param connection The associated TCP connection to receive any response
	 * @retval bool true if command handled and response sent
	 */
	virtual bool onCommand(String cmd, String data, FtpServerConnection& connection)
	{
		return false;
	}

	IFS::FileSystem* getFileSystem() const
	{
		return fileSystem ?: ::getFileSystem();
	}

private:
	IFS::FileSystem* fileSystem;
};

/**
 *  @ingroup    ftpserver
 *  @brief      Provides FTP server
 */
class FtpServer : public CustomFtpServer
{
public:
	void addUser(const String& login, const String& pass, IFS::UserRole userRole = IFS::UserRole::Admin);
	IFS::UserRole validateUser(const char* login, const char* pass) override;

protected:
	bool onCommand(String cmd, String data, FtpServerConnection& connection) override;

private:
	struct User {
		String password;
		IFS::UserRole role;
	};
	using UserList = HashMap<String, User>;
	UserList users;
};
