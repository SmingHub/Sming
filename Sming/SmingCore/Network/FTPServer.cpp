/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "FTPServer.h"

#include "Http/HttpRequest.h"
#include "Http/HttpResponse.h"
#include "FTPServerConnection.h"
#include "TcpClient.h"
#include "WString.h"

FTPServer::FTPServer()
{
	setTimeOut(900); // Update timeout
}

FTPServer::~FTPServer()
{}

TcpConnection* FTPServer::createClient(tcp_pcb* clientTcp)
{
	TcpConnection* con = new FTPServerConnection(this, clientTcp);
	return con;
}

void FTPServer::addUser(const String& login, const String& pass)
{
	debug_d("addUser: %s %s", login.c_str(), pass.c_str());
	_users[login] = pass;
}

bool FTPServer::checkUser(const String& login, const String& pass)
{
	debug_d("checkUser: %s %s", login.c_str(), pass.c_str());
	if (!_users.contains(login))
		return false;

	return _users[login] == pass;
}

bool FTPServer::onCommand(String cmd, String data, FTPServerConnection& connection)
{
	if (cmd == "FSFORMAT") {
		int err = fileSystemFormat();
		connection.response(200, F("File system format returned ") + String(err));
		return true;
	}
	return false;
}
