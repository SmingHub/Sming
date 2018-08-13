
#include "FTPServerConnection.h"
#include "FTPServer.h"
#include "NetUtils.h"
#include "TcpConnection.h"
#include "FileSystem.h"
#include "../Services/DateTime/DateTime.h"

/*
 * @todo There are a lot of classes defined in this file. Create an 'FTP' folder
 * and move everything in there. Only the FTPServer module belongs in this folder.
 */

/* FTPDataStream */

class FTPDataStream : public TcpConnection {
public:
	FTPDataStream(FTPServerConnection* connection) : TcpConnection(true), _parent(connection)
	{}

	virtual err_t onConnected(err_t err)
	{
		//response(125, "Connected");
		setTimeOut(300); // Update timeout
		return TcpConnection::onConnected(err);
	}

	virtual err_t onSent(uint16_t len)
	{
		_sent += len;

		if (_completed && _sent >= _written)
			finishTransfer();

		return TcpConnection::onSent(len);
	}

	void finishTransfer()
	{
		close();
		_parent->dataTransferFinished(this);
	}

	void response(int code, const String& text = nullptr)
	{
		_parent->response(code, text);
	}

	int write(const char* data, int len, uint8_t apiflags = TCP_WRITE_FLAG_COPY)
	{
		_written += len;
		return TcpConnection::write(data, len, apiflags);
	}

	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent)
	{
		if (!_parent->isCanTransfer())
			return;
		if (_completed && _written == 0)
			finishTransfer();
		transferData(sourceEvent);
	}

	virtual void transferData(TcpConnectionEvent sourceEvent)
	{}

protected:
	FTPServerConnection* _parent = nullptr;
	bool _completed = false;
	int _written = 0;
	int _sent = 0;
};

/* FTPDataFileList */

class FTPDataFileList : public FTPDataStream {
public:
	FTPDataFileList(FTPServerConnection* connection) : FTPDataStream(connection)
	{}

	virtual void transferData(TcpConnectionEvent sourceEvent);
};

/*
 * 13/8/2018 (mikee47)
 *
 * 	Rewritten using new filesystem api: we now have real file timestamps
 */
void FTPDataFileList::transferData(TcpConnectionEvent sourceEvent)
{
	if (_completed)
		return;

	Vector<String> list = fileList();
	debug_d("send file list: %d", list.count());
	for (int unsigned i = 0; i < list.count(); i++)
		writeString("01-01-15  01:00AM               " + String(fileGetSize(list[i])) + " " + list[i] + "\r\n");

	_completed = true;
	finishTransfer();
}

/* FTPDataRetrieve */

class FTPDataRetrieve : public FTPDataStream {
public:
	FTPDataRetrieve(FTPServerConnection* connection, const String& fileName) : FTPDataStream(connection)
	{
		_file = fileOpen(fileName, eFO_ReadOnly);
	}

	~FTPDataRetrieve()
	{
		fileClose(_file);
	}

	virtual void transferData(TcpConnectionEvent sourceEvent)
	{
		if (_completed)
			return;
		char buf[1024];
		int len = fileRead(_file, buf, sizeof(buf));
		write(buf, len);
		if (fileIsEOF(_file)) {
			_completed = true;
			finishTransfer();
		}
	}

private:
	file_t _file = -1;
};

/* FTPDataStore */

class FTPDataStore : public FTPDataStream {
public:
	FTPDataStore(FTPServerConnection* connection, const String& fileName) : FTPDataStream(connection)
	{
		file = fileOpen(fileName, eFO_WriteOnly | eFO_CreateNewAlways);
	}

	~FTPDataStore()
	{
		fileClose(file);
	}

	virtual err_t onReceive(pbuf* buf);

private:
	file_t file;
};

err_t FTPDataStore::onReceive(pbuf* buf)
{
	if (!_completed) {
		if (!buf) {
			_completed = true;
			response(226, _F("Transfer completed"));
		}
		else {
			for (pbuf* cur = buf; cur && cur->len > 0; cur = cur->next)
				fileWrite(file, (uint8_t*)cur->payload, cur->len);
		}
	}

	return TcpConnection::onReceive(buf);
}

/* FTPServerConnection */

err_t FTPServerConnection::onReceive(pbuf* buf)
{
	if (!buf)
		return ERR_OK;

	int p = 0, prev = 0;

	while (true) {
		p = NetUtils::pbufFindStr(buf, "\r\n", prev);
		if (p < 0 || p - prev > MAX_FTP_CMD)
			break;
		int split = NetUtils::pbufFindChar(buf, ' ', prev);
		String cmd, data;
		if (split >= 0 && split < p) {
			cmd = NetUtils::pbufStrCopy(buf, prev, split - prev);
			split++;
			data = NetUtils::pbufStrCopy(buf, split, p - split);
		}
		else
			cmd = NetUtils::pbufStrCopy(buf, prev, p - prev);
		debug_d("%s: '%s'", cmd.c_str(), data.c_str());
		onCommand(cmd, data);
		prev = p + 1;
	}

	return ERR_OK;
}

void FTPServerConnection::cmdPort(const String& data)
{
	int last = getSplitterPos(data, ',', 4);
	if (last < 0) {
		response(550); // Invalid arguments
					   //return;
	}
	int ipEnd = getSplitterPos(data, ',', 3);
	String sip = data.substring(0, ipEnd);
	sip.replace(',', '.');
	_ip = sip;
	String ps1 = data.substring(ipEnd + 1, last);
	String ps2 = data.substring(last + 1);
	int p1 = ps1.toInt();
	int p2 = ps2.toInt();
	_port = (p1 << 8) | p2;
	debug_d("connection to: %s, %d", _ip.toString().c_str(), _port);
	response(200);
}

void FTPServerConnection::onCommand(const String& cmd, const String& data)
{
#define CMD_IS(_tag) cmd.equalsIgnoreCase(_F(_tag))

	// We ready to quit always :)
	if (CMD_IS("QUIT")) {
		response(221);
		close();
		return;
	}

	// Strong security check :)
	if (_state == eFCS_Authorization) {
		if (CMD_IS("USER")) {
			_userName = data;
			response(331);
		}
		else if (CMD_IS("PASS")) {
			if (_server->checkUser(_userName, data)) {
				_userName = nullptr;
				_state = eFCS_Active;
				response(230);
			}
			else
				response(430);
		}
		else
			response(530);

		return;
	}

	if (_state == eFCS_Active) {
		if (CMD_IS("SYST")) {
			response(215, F("Windows_NT: Sming Framework")); // Why not? It's look like Windows :)
		}
		else if (CMD_IS("PWD")) {
			response(257, "\"/\"");
		}
		else if (CMD_IS("PORT")) {
			cmdPort(data);
		}
		else if (CMD_IS("CWD")) {
			if (data[0] == '/' && data.length() == 1)
				response(250);
			else
				response(550);
		}
		else if (CMD_IS("TYPE")) {
			response(250);
		}
		/*else if CMD_IS("SIZE") {
			response(213, String(fileGetSize(makeFileName(data, false))));
		}*/
		else if (CMD_IS("DELE")) {
			String name = makeFileName(data, false);
			response(fileDelete(name) >= 0 ? 250 : 550);
		}
		/*else if CMD_IS("RNFR") { // Bugs!
			renameFrom = data;
			response(350);
		}
		else if (CMD_IS("RNTO")) {
			response(fileRename(renameFrom, data) >= 0 ? 250 : 550);
		}*/
		else if (CMD_IS("RETR")) {
			createDataConnection(new FTPDataRetrieve(this, makeFileName(data, false)));
		}
		else if (CMD_IS("STOR")) {
			createDataConnection(new FTPDataStore(this, makeFileName(data, true)));
		}
		else if (CMD_IS("LIST")) {
			createDataConnection(new FTPDataFileList(this));
		}
		else if (CMD_IS("PASV")) {
			response(500, F("Passive mode not supported"));
		}
		else if (CMD_IS("NOOP")) {
			response(200);
		}
		else if (!_server->onCommand(cmd, data, *this))
			response(502, F("Not supported"));

		return;
	}

#undef CMD_IS

	debug_e("!!!CASE NOT IMPLEMENTED?!!!");
}

err_t FTPServerConnection::onSent(uint16_t len)
{
	_canTransfer = true;
	return ERR_OK;
}

String FTPServerConnection::makeFileName(String name, bool shortIt)
{
	if (name[0] == '/')
		name.remove(0, 1);

	if (!shortIt || name.length() <= 20)
		return name;

	String ext;
	int i = name.lastIndexOf('.');
	if (i >= 0)
		ext = name.substring(i);

	return name.substring(0, 16) + ext;
}

void FTPServerConnection::createDataConnection(TcpConnection* connection)
{
	_dataConnection = connection;
	_dataConnection->connect(_ip, _port);
	response(150, "Connecting");
}

void FTPServerConnection::dataTransferFinished(TcpConnection* connection)
{
	if (connection != _dataConnection)
		SYSTEM_ERROR("FTP Wrong state: connection != dataConnection");

	_dataConnection = nullptr;
	response(226, "Transfer Complete.");
}

int FTPServerConnection::getSplitterPos(const String& data, char splitter, uint8_t number)
{
	uint8_t k = 0;

	for (unsigned i = 0; i < data.length(); i++) {
		if (data[i] == splitter) {
			if (k == number)
				return i;
			k++;
		}
	}

	return -1;
}

void FTPServerConnection::response(int code, const String& text)
{
	String response = String(code, DEC);
	if (text.length() == 0) {
		if (code >= 200 && code <= 399) // Just for simplicity
			response += _F(" OK");
		else
			response += _F(" FAIL");
	}
	else
		response += " " + text;
	response += "\r\n";

	debug_d("> %s", response.c_str());
	writeString(response);
	_canTransfer = false;
	flush();
}

void FTPServerConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	if (_state == eFCS_Ready) {
		writeString(_F("220 Welcome to Sming FTP\r\n"));
		_state = eFCS_Authorization;
	}
}
