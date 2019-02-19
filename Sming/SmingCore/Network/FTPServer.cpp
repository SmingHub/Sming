/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpServer.cpp
 *
 ****/

#include "FTPServer.h"

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "FTPServerConnection.h"
#include "TcpClient.h"
#include "WString.h"

FTPServer::FTPServer()
{
	setTimeOut(900); // Update timeout
}

FTPServer::~FTPServer()
{
}

TcpConnection* FTPServer::createClient(tcp_pcb* clientTcp)
{
	TcpConnection* con = new FTPServerConnection(this, clientTcp);
	return con;
}

void FTPServer::addUser(const String& login, const String& pass)
{
	debug_d("addUser: %s %s", login.c_str(), pass.c_str());
	users[login] = pass;
}

bool FTPServer::checkUser(const String& login, const String& pass)
{
	debug_d("checkUser: %s %s", login.c_str(), pass.c_str());
	if(!users.contains(login))
		return false;

	return users[login] == pass;
}

bool FTPServer::onCommand(String cmd, String data, FTPServerConnection& connection)
{
	if(cmd == "FSFORMAT") {
		spiffs_format();
		connection.response(200, F("File system successfully formatted"));
		return true;
	}
	return false;
}
