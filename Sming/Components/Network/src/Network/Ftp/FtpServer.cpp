/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpServer.cpp
 *
 ****/

#include "FtpServer.h"
#include "FtpServerConnection.h"

TcpConnection* CustomFtpServer::createClient(tcp_pcb* clientTcp)
{
	return new FtpServerConnection(*this, clientTcp);
}

void FtpServer::addUser(const String& login, const String& pass, IFS::UserRole role)
{
	debug_d("addUser: %s %s (%s)", login.c_str(), pass.c_str(), toString(role).c_str());
	users[login] = User{pass, role};
}

IFS::UserRole FtpServer::validateUser(const char* login, const char* pass)
{
	debug_d("validateUser: %s %s", login, pass);
	auto& user = static_cast<const UserList&>(users)[login];
	return (user.password == pass) ? user.role : IFS::UserRole::None;
}

bool FtpServer::onCommand(String cmd, String data, FtpServerConnection& connection)
{
	if(cmd == _F("FSFORMAT")) {
		auto fs = connection.getFileSystem();
		if(fs != nullptr) {
			int err = fs->format();
			connection.response(200, F("File system format: ") + fileGetErrorString(err));
		}
		return true;
	}
	return false;
}
