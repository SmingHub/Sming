/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpServerConnection.cpp
 *
 ****/

#include "FtpServerConnection.h"
#include "FtpDataStore.h"
#include "FtpDataRetrieve.h"
#include "FtpDataFileList.h"
#include "../FtpServer.h"
#include "Network/NetUtils.h"

err_t FtpServerConnection::onReceive(pbuf* buf)
{
	if(buf == nullptr) {
		return ERR_OK;
	}
	int prev = 0;

	while(true) {
		int p = NetUtils::pbufFindStr(buf, "\r\n", prev);
		if(p < 0 || p - prev > MAX_FTP_CMD) {
			break;
		}
		int split = NetUtils::pbufFindChar(buf, ' ', prev);
		String cmd, data;
		if(split >= 0 && split < p) {
			cmd = NetUtils::pbufStrCopy(buf, prev, split - prev);
			split++;
			data = NetUtils::pbufStrCopy(buf, split, p - split);
		} else {
			cmd = NetUtils::pbufStrCopy(buf, prev, p - prev);
		}
		debug_d("%s: '%s'", cmd.c_str(), data.c_str());
		onCommand(cmd, data);
		prev = p + 1;
	}

	return ERR_OK;
}

void FtpServerConnection::cmdPort(const String& data)
{
	int last = getSplitterPos(data, ',', 4);
	if(last < 0) {
		response(550); // Invalid arguments
					   //return;
	}
	int ipEnd = getSplitterPos(data, ',', 3);
	String sip = data.substring(0, ipEnd);
	sip.replace(',', '.');
	ip = sip;
	String ps1 = data.substring(ipEnd + 1, last);
	String ps2 = data.substring(last + 1);
	int p1 = ps1.toInt();
	int p2 = ps2.toInt();
	port = (p1 << 8) | p2;
	debug_d("connection to: %s, %d", ip.toString().c_str(), port);
	response(200);
}

void FtpServerConnection::onCommand(String cmd, String data)
{
	cmd.toUpperCase();
	// We ready to quit always :)
	if(cmd == _F("QUIT")) {
		response(221);
		close();
		return;
	}

	// Strong security check :)
	if(state == eFCS_Authorization) {
		if(cmd == _F("USER")) {
			userName = data;
			response(331);
		} else if(cmd == _F("PASS")) {
			if(server->checkUser(userName, data)) {
				userName = "";
				state = eFCS_Active;
				response(230);
			} else
				response(430);
		} else {
			response(530);
		}
		return;
	}

	if(state == eFCS_Active) {
		if(cmd == _F("SYST")) {
			response(215, F("Windows_NT: Sming Framework")); // Why not? It's look like Windows :)
		} else if(cmd == _F("PWD")) {
			response(257, F("\"/\""));
		} else if(cmd == _F("PORT")) {
			cmdPort(data);
		} else if(cmd == _F("CWD")) {
			if(data == "/")
				response(250);
			else
				response(550);
		} else if(cmd == _F("TYPE")) {
			response(250);
		}
		/*else if (cmd == _F("SIZE"))
		{
			response(213, String(fileGetSize(makeFileName(data, false))));
		}*/
		else if(cmd == _F("DELE")) {
			String name = makeFileName(data, false);
			if(fileExist(name)) {
				fileDelete(name);
				response(250);
			} else
				response(550);
		}
		/*else if (cmd == _F("RNFR")) // Bugs!
		{
			renameFrom = data;
			response(350);
		}
		else if (cmd == _F("RNTO"))
		{
			if (fileExist(renameFrom))
			{
				fileRename(renameFrom, data);
				response(250);
			}
			else
				response(550);
		}*/
		else if(cmd == _F("RETR")) {
			createDataConnection(new FtpDataRetrieve(this, makeFileName(data, false)));
		} else if(cmd == _F("STOR")) {
			createDataConnection(new FtpDataStore(this, makeFileName(data, true)));
		} else if(cmd == _F("LIST")) {
			createDataConnection(new FtpDataFileList(this));
		} else if(cmd == _F("PASV")) {
			response(500, F("Passive mode not supported"));
		} else if(cmd == _F("NOOP")) {
			response(200);
		} else if(!server->onCommand(cmd, data, *this))
			response(502, F("Not supported"));

		return;
	}

	debug_e("!!!CASE NOT IMPLEMENTED?!!!");
}

err_t FtpServerConnection::onSent(uint16_t len)
{
	canTransfer = true;

	return ERR_OK;
}

String FtpServerConnection::makeFileName(String name, bool shortIt)
{
	if(name.startsWith("/")) {
		name.remove(0, 1);
	}

	if(shortIt && name.length() > 20) {
		String ext;
		int dotPos = name.lastIndexOf('.');
		if(dotPos >= 0) {
			ext = name.substring(dotPos);
		}

		return name.substring(0, 16) + ext;
	}
	return name;
}

void FtpServerConnection::createDataConnection(TcpConnection* connection)
{
	dataConnection = connection;
	dataConnection->connect(ip, port);
	response(150, F("Connecting"));
}

void FtpServerConnection::dataTransferFinished(TcpConnection* connection)
{
	if(connection != dataConnection) {
		SYSTEM_ERROR("FTP Wrong state: connection != dataConnection");
	}

	dataConnection = nullptr;
	response(226, F("Transfer Complete."));
}

int FtpServerConnection::getSplitterPos(const String& data, char splitter, uint8_t number)
{
	uint8_t k = 0;

	for(unsigned i = 0; i < data.length(); i++) {
		if(data[i] == splitter) {
			if(k == number) {
				return i;
			}
			k++;
		}
	}

	return -1;
}

void FtpServerConnection::response(int code, String text /* = "" */)
{
	String response = String(code, DEC);
	if(text.length() == 0) {
		if(code >= 200 && code <= 399) { // Just for simplify
			response += _F(" OK");
		} else {
			response += _F(" FAIL");
		}
	} else {
		response += ' ' + text;
	}
	response += "\r\n";

	debug_d("> %s", response.c_str());
	writeString(response, TCP_WRITE_FLAG_COPY); // Dynamic memory, should copy
	canTransfer = false;
	flush();
}

void FtpServerConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	switch(state) {
	case eFCS_Ready:
		writeString(_F("220 Welcome to Sming FTP\r\n"), 0);
		state = eFCS_Authorization;
		break;

	default:; // Do nothing
	}
}
