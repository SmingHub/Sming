#include "FTPServerConnection.h"
#include "FTPServer.h"
#include "NetUtils.h"
#include "TcpConnection.h"
#include "../FileSystem.h"

class FTPDataStream : public TcpConnection
{
public:
	FTPDataStream(FTPServerConnection* connection) : TcpConnection(true), parent(connection), completed(false), sent(0), written(0) {}
	virtual err_t onConnected(err_t err)
	{
		//response(125, "Connected");
		setTimeOut(300); // Update timeout
		return TcpConnection::onConnected(err);
	}
	virtual err_t onSent(uint16_t len)
	{
		sent += len;
		if (written < sent || !completed) return TcpConnection::onSent(len);
		finishTransfer();
		return TcpConnection::onSent(len);
	}
	void finishTransfer()
	{
		close();
		parent->dataTransferFinished(this);
	}
	void response(int code, String text = "") { parent->response(code, text); }
	int write(const char* data, int len, uint8_t apiflags = 0)
	{
		written += len;
		return TcpConnection::write(data, len, apiflags);
	}
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent)
	{
		if (!parent->isCanTransfer()) return;
		if (completed && written == 0) finishTransfer();
		transferData(sourceEvent);
	}
	virtual void transferData(TcpConnectionEvent sourceEvent) {}

protected:
	FTPServerConnection* parent;
	bool completed;
	int written;
	int sent;
};

class FTPDataFileList : public FTPDataStream
{
public:
	FTPDataFileList(FTPServerConnection* connection) : FTPDataStream(connection) {}
	virtual void transferData(TcpConnectionEvent sourceEvent)
	{
		if (completed) return;
		Vector<String> list = fileList();
		debugf("send file list: %d", list.count());
		for (int i = 0; i < list.count(); i++)
			writeString("01-01-15  01:00AM               " + String(fileGetSize(list[i])) + " " + list[i] + "\r\n");
		completed = true;
	}
};

class FTPDataRetrieve : public FTPDataStream
{
public:
	FTPDataRetrieve(FTPServerConnection* connection, String fileName) : FTPDataStream(connection)
	{
		file = fileOpen(fileName, eFO_ReadOnly);
	}
	~FTPDataRetrieve()
	{
		fileClose(file);
	}
	virtual void transferData(TcpConnectionEvent sourceEvent)
	{
		if (completed) return;
		int p = fileTell(file);
		if (p == 0)
			response(250, "Transfer started");

		char buf[256];
		int len = fileRead(file, buf, 256);
		write(buf, len, TCP_WRITE_FLAG_COPY);

		if (fileIsEOF(file))
		{
			completed = true;
			response(226, "Transfer completed");
		}
	}

private:
	file_t file;
};

class FTPDataStore : public FTPDataStream
{
public:
	FTPDataStore(FTPServerConnection* connection, String fileName) : FTPDataStream(connection)
	{
		file = fileOpen(fileName, eFO_WriteOnly | eFO_CreateNewAlways);
	}
	~FTPDataStore()
	{
		fileClose(file);
	}
	virtual err_t onReceive(pbuf *buf)
	{
		if (completed) return TcpConnection::onReceive(buf);

		if (buf == NULL)
		{
			completed = true;
			response(226, "Transfer completed");
			return TcpConnection::onReceive(buf);
		}
		int p = fileTell(file);
		if (p == 0)
			response(250, "Transfer started");

		pbuf *cur = buf;
		while (cur)
		{
			int len = fileWrite(file, cur->payload, cur->len);
			cur = cur->next;
		}

		return TcpConnection::onReceive(buf);
	}

private:
	file_t file;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

FTPServerConnection::FTPServerConnection(FTPServer *parentServer, tcp_pcb *clientTcp)
	: TcpConnection(clientTcp, true), server(parentServer), state(eFCS_Ready)
{
	dataConnection = NULL;
	port = 0;
	canTransfer = true;
}

FTPServerConnection::~FTPServerConnection()
{
}

err_t FTPServerConnection::onReceive(pbuf *buf)
{
	if (buf == NULL) return ERR_OK;
	int p = 0, prev = 0;

	while (true)
	{
		p = NetUtils::pbufFindStr(buf, "\r\n", prev);
		if (p == -1 || p - prev > MAX_FTP_CMD) break;
		int split = NetUtils::pbufFindChar(buf, ' ', prev);
		String cmd, data;
		if (split != -1 && split < p)
		{
			cmd = NetUtils::pbufStrCopy(buf, prev, split - prev);
			split++;
			data = NetUtils::pbufStrCopy(buf, split, p - split);
		}
		else
			cmd = NetUtils::pbufStrCopy(buf, prev, p - prev);
		debugf("%s: '%s'", cmd.c_str(), data.c_str());
		onCommand(cmd, data);
		prev = p + 1;
	};
	return ERR_OK;
}

void FTPServerConnection::cmdPort(const String& data)
{
	int last = getSplitterPos(data, ',', 4);
	if (last == -1)
	{
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
	debugf("connection to: %s, %d", ip.toString().c_str(), port);
	response(200);
}

void FTPServerConnection::onCommand(String cmd, String data)
{
	cmd.toUpperCase();
	// We ready to quit always :)
	if (cmd == "QUIT")
	{
		response(221);
		close();
		return;
	}

	// Strong security check :)
	if (state == eFCS_Authorization)
	{
		if (cmd == "USER")
		{
			userName = data;
			response(331);
		}
		else if (cmd == "PASS")
		{
			if (server->checkUser(userName, data))
			{
				userName = "";
				state = eFCS_Active;
				response(230);
			}
			else
				response(430);
		}
		else
		{
			response(530);
		}
		return;
	}

	if (state == eFCS_Active)
	{
		if (cmd == "SYST")
		{
			response(215, "Windows_NT: Sming Framework"); // Why not? It's look like Windows :)
		}
		else if (cmd == "PWD")
		{
			response(257, "\"/\"");
		}
		else if (cmd == "PORT")
		{
			cmdPort(data);
		}
		else if (cmd == "CWD")
		{
			if (data == "/")
				response(250);
			else
				response(550);
		}
		else if (cmd == "TYPE")
		{
			response(250);
		}
		/*else if (cmd == "SIZE")
		{
			response(213, String(fileGetSize(makeFileName(data, false))));
		}*/
		else if (cmd == "DELE")
		{
			String name = makeFileName(data, false);
			if (fileExist(name))
			{
				fileDelete(name);
				response(250);
			}
			else
				response(550);
		}
		/*else if (cmd == "RNFR") // Bugs!
		{
			renameFrom = data;
			response(350);
		}
		else if (cmd == "RNTO")
		{
			if (fileExist(renameFrom))
			{
				fileRename(renameFrom, data);
				response(250);
			}
			else
				response(550);
		}*/
		else if (cmd == "RETR")
		{
			createDataConnection(new FTPDataRetrieve(this, makeFileName(data, false)));
		}
		else if (cmd == "STOR")
		{
			createDataConnection(new FTPDataStore(this, makeFileName(data, true)));
		}
		else if (cmd == "LIST")
		{
			createDataConnection(new FTPDataFileList(this));
		}
		else if (cmd == "PASV")
		{
			response(500 , "Passive mode not supported");
		}
		else if (cmd == "NOOP")
		{
			response(200);
		}
		else if (!server->onCommand(cmd, data, *this))
			response(502, "Not supported");

		return;
	}

	debugf("!!!CASE NOT IMPLEMENTED?!!!");
}

err_t FTPServerConnection::onSent(uint16_t len)
{
	canTransfer = true;
}

String FTPServerConnection::makeFileName(String name, bool shortIt)
{
	if (name.startsWith("/"))
		name = name.substring(1);

	if (shortIt && name.length() > 20)
	{
		String ext = "";
		if (name.lastIndexOf('.') != -1)
			ext = name.substring(name.lastIndexOf('.'));

		return name.substring(0, 16) + ext;
	}
	return name;
}

void FTPServerConnection::createDataConnection(TcpConnection* connection)
{
	dataConnection = connection;
	dataConnection->connect(ip, port);
	response(150, "Connecting");
}

void FTPServerConnection::dataTransferFinished(TcpConnection* connection)
{
	if (connection != dataConnection)
		SYSTEM_ERROR("FTP Wrong state: connection != dataConnection");

	dataConnection = NULL;
	response(226, "Transfer Complete.");
}

int FTPServerConnection::getSplitterPos(String data, char splitter, uint8_t number)
{
	uint8_t k = 0;

	for (int i = 0; i < data.length(); i++)
	{
		if (data[i] == splitter)
		{
			if (k == number)
			{
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
	if (text.length() == 0)
	{
		if (code >= 200 && code <= 399) // Just for simplify
			response += " OK";
		else
			response += " FAIL";
	}
	else
		response += " " + text;
	response += "\r\n";

	debugf("> %s", response.c_str());
	writeString(response.c_str(), TCP_WRITE_FLAG_COPY); // Dynamic memory, should copy
	canTransfer = false;
	flush();
}

void FTPServerConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	switch (state)
	{
		case eFCS_Ready:
			this->writeString("220 Welcome to Sming FTP\r\n", 0);
			state = eFCS_Authorization;
			break;
	}
}
