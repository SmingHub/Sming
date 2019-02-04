#include "FTPServerConnection.h"
#include "FTPServer.h"
#include "NetUtils.h"
#include "TcpConnection.h"
#include "../FileSystem.h"

class FTPDataStream : public TcpConnection
{
public:
	FTPDataStream(FTPServerConnection* connection) : TcpConnection(true), parent(connection)
	{
	}

	virtual err_t onConnected(err_t err)
	{
		//response(125, "Connected");
		setTimeOut(300); // Update timeout
		return TcpConnection::onConnected(err);
	}

	virtual err_t onSent(uint16_t len)
	{
		sent += len;
		if(written < sent || !completed) {
			return TcpConnection::onSent(len);
		}
		finishTransfer();
		return TcpConnection::onSent(len);
	}

	void finishTransfer()
	{
		close();
		parent->dataTransferFinished(this);
	}

	void response(int code, String text = nullptr)
	{
		parent->response(code, text);
	}

	int write(const char* data, int len, uint8_t apiflags = 0)
	{
		written += len;
		return TcpConnection::write(data, len, apiflags);
	}

	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent)
	{
		if(!parent->isCanTransfer()) {
			return;
		}
		if(completed && written == 0) {
			finishTransfer();
		}
		transferData(sourceEvent);
	}

	virtual void transferData(TcpConnectionEvent sourceEvent)
	{
	}

protected:
	FTPServerConnection* parent = nullptr;
	bool completed = false;
	unsigned written = 0;
	unsigned sent = 0;
};

class FTPDataFileList : public FTPDataStream
{
public:
	FTPDataFileList(FTPServerConnection* connection) : FTPDataStream(connection)
	{
	}

	virtual void transferData(TcpConnectionEvent sourceEvent)
	{
		if(completed) {
			return;
		}
		Vector<String> list = fileList();
		debug_d("send file list: %d", list.count());
		for(unsigned i = 0; i < list.count(); i++) {
			writeString("01-01-15  01:00AM               " + String(fileGetSize(list[i])) + " " + list[i] + "\r\n");
		}
		completed = true;
		finishTransfer();
	}
};

class FTPDataRetrieve : public FTPDataStream
{
public:
	FTPDataRetrieve(FTPServerConnection* connection, const String& fileName) : FTPDataStream(connection)
	{
		file = fileOpen(fileName, eFO_ReadOnly);
	}

	~FTPDataRetrieve()
	{
		fileClose(file);
	}

	virtual void transferData(TcpConnectionEvent sourceEvent)
	{
		if(completed) {
			return;
		}
		char buf[1024];
		int len = fileRead(file, buf, 1024);
		write(buf, len, TCP_WRITE_FLAG_COPY);
		if(fileIsEOF(file)) {
			completed = true;
			finishTransfer();
		}
	}

private:
	file_t file;
};

class FTPDataStore : public FTPDataStream
{
public:
	FTPDataStore(FTPServerConnection* connection, const String& fileName) : FTPDataStream(connection)
	{
		file = fileOpen(fileName, eFO_WriteOnly | eFO_CreateNewAlways);
	}

	~FTPDataStore()
	{
		fileClose(file);
	}

	virtual err_t onReceive(pbuf* buf)
	{
		if(completed) {
			return TcpConnection::onReceive(buf);
		}

		if(buf == nullptr) {
			completed = true;
			response(226, "Transfer completed");
			return TcpConnection::onReceive(buf);
		}

		pbuf* cur = buf;
		while(cur != nullptr && cur->len > 0) {
			fileWrite(file, (uint8_t*)cur->payload, cur->len);
			cur = cur->next;
		}

		return TcpConnection::onReceive(buf);
	}

private:
	file_t file;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

err_t FTPServerConnection::onReceive(pbuf* buf)
{
	if(buf == nullptr)
		return ERR_OK;
	int p = 0, prev = 0;

	while(true) {
		p = NetUtils::pbufFindStr(buf, "\r\n", prev);
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

void FTPServerConnection::cmdPort(const String& data)
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

void FTPServerConnection::onCommand(String cmd, String data)
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
			createDataConnection(new FTPDataRetrieve(this, makeFileName(data, false)));
		} else if(cmd == _F("STOR")) {
			createDataConnection(new FTPDataStore(this, makeFileName(data, true)));
		} else if(cmd == _F("LIST")) {
			createDataConnection(new FTPDataFileList(this));
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

err_t FTPServerConnection::onSent(uint16_t len)
{
	canTransfer = true;

	return ERR_OK;
}

String FTPServerConnection::makeFileName(String name, bool shortIt)
{
	if(name.startsWith("/")) {
		name = name.substring(1);
	}

	if(shortIt && name.length() > 20) {
		String ext = "";
		if(name.lastIndexOf('.') != -1) {
			ext = name.substring(name.lastIndexOf('.'));
		}

		return name.substring(0, 16) + ext;
	}
	return name;
}

void FTPServerConnection::createDataConnection(TcpConnection* connection)
{
	dataConnection = connection;
	dataConnection->connect(ip, port);
	response(150, F("Connecting"));
}

void FTPServerConnection::dataTransferFinished(TcpConnection* connection)
{
	if(connection != dataConnection) {
		SYSTEM_ERROR("FTP Wrong state: connection != dataConnection");
	}

	dataConnection = nullptr;
	response(226, F("Transfer Complete."));
}

int FTPServerConnection::getSplitterPos(String data, char splitter, uint8_t number)
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

void FTPServerConnection::response(int code, String text /* = "" */)
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

void FTPServerConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	switch(state) {
	case eFCS_Ready:
		this->writeString(_F("220 Welcome to Sming FTP\r\n"), 0);
		state = eFCS_Authorization;
		break;

	default:; // Do nothing
	}
}
