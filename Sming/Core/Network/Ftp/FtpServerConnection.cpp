/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
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
#include <Data/CStringArray.h>

// Name, Comment
#define FTP_COMMAND_MAP(XX)                                                                                            \
	XX(ACCT, "Identifies user's account")                                                                              \
	XX(CWD, "Change working directory")                                                                                \
	XX(DELE, "Delete file")                                                                                            \
	XX(LIST, "List file or directory information")                                                                     \
	XX(NLST, "List file or directory names")                                                                           \
	XX(NOOP, "")                                                                                                       \
	XX(PWD, "Get current working directory")                                                                           \
	XX(PASS, "Must follow user")                                                                                       \
	XX(PASV, "")                                                                                                       \
	XX(PORT, "")                                                                                                       \
	XX(QUIT, "")                                                                                                       \
	XX(RNFR, "Rename file: FROM")                                                                                      \
	XX(RNTO, "Rename file: TO")                                                                                        \
	XX(RETR, "Retrieve file content")                                                                                  \
	XX(SIZE, "Get file size")                                                                                          \
	XX(STOR, "Store file data")                                                                                        \
	XX(SYST, "Get system information")                                                                                 \
	XX(TYPE, "")                                                                                                       \
	XX(USER, "Start login sequence, flushing any current account information")

namespace
{
#define XX(name, comment) #name "\0"
DEFINE_FSTR(fstrCommandList, FTP_COMMAND_MAP(XX))
#undef XX

enum class Command {
	UNKNOWN,
#define XX(name, comment) name,
	FTP_COMMAND_MAP(XX)
#undef XX
};

Command parseCommand(const String& data)
{
	int i = CStringArray(fstrCommandList).indexOf(data);
	return Command(i + 1);
}

int getSplitterPos(const String& data, char splitter, uint8_t number)
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

String resolvePath(const String& cwd, const String& name)
{
	if(name[0] == '/') {
		return name.substring(1);
	}

	String path;
	path.reserve(cwd.length() + name.length() + 1);
	path += cwd;
	if(name.length() != 0) {
		if(path.length() != 0) {
			path += '/';
		}
		path += name;
	}
	// Remove any trailing path separator
	auto len = path.length();
	if(path[len - 1] == '/') {
		path.setLength(len - 1);
	}
	return path;
}

} // namespace

FtpServerConnection::FtpServerConnection(CustomFtpServer& parentServer, tcp_pcb* clientTcp)
	: TcpConnection(clientTcp, true), server(parentServer)
{
	writeString(_F("220 Welcome to Sming FTP\r\n"));
}

err_t FtpServerConnection::onReceive(pbuf* buf)
{
	if(buf == nullptr) {
		return ERR_OK;
	}
	int prev = 0;

	while(true) {
		int p = NetUtils::pbufFindStr(buf, "\r\n", prev);
		if(p < 0 || size_t(p - prev) > MAX_FTP_CMD) {
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

bool FtpServerConnection::checkFileAccess(const String& filename, FileOpenFlags flags)
{
	FileStat stat;
	int err = fileStats(filename, stat);
	if(err == IFS::Error::NotFound && flags[FileOpenFlag::Write]) {
		// File doesn't exist - check permissions on parent directory
		String path;
		int i = filename.lastIndexOf('/');
		if(i > 0) {
			path = filename.substring(0, i);
		}
		err = fileStats(path, stat);
	}
	if(err != FS_OK) {
		response(550, fileGetErrorString(err)); // Not found / no access
		return false;
	}
	if(flags[FileOpenFlag::Write]) {
		if(stat.attr[FileAttribute::ReadOnly]) {
			response(550, fileGetErrorString(IFS::Error::ReadOnly));
		}
		if(user.role < stat.acl.writeAccess) {
			response(550, _F("Write access denied")); // Not found / no access
			return false;
		}
	}
	if(user.role < stat.acl.readAccess) {
		response(550, _F("Read access denied")); // Not found / no access
		return false;
	}
	return true;
}

void FtpServerConnection::onCommand(String cmd, String data)
{
	auto command = parseCommand(cmd);
	switch(command) {
	// We ready to quit always :)
	case Command::QUIT:
		response(221); // Service closing control connection
		close();
		break;

	// Strong security check :)
	case Command::USER:
		// Authenticate or re-authenticate, wiping existing credentials
		user = User{data};
		response(331); // User name OK, need password
		break;

	case Command::PASS: {
		if(!user.name) {
			response(332); // Need account for login
		} else {
			user.role = server.validateUser(user.name, data);
			response(user.isValid() ? 230 : 430);
		}
		user.name = nullptr;
		break;
	}

	case Command::SYST:
		response(215, F("Windows_NT: Sming Framework")); // Why not? It's look like Windows :)
		break;

	case Command::PWD: {
		String s;
		s += "\"/";
		s += cwd;
		s += '"';
		response(257, s);
		break;
	}

	case Command::PORT:
		cmdPort(data);
		break;

	case Command::CWD: {
		String path = resolvePath(cwd, data);
		if(!checkFileAccess(path, FileOpenFlag::Read)) {
			break;
		}
		debug_i("CWD: '%s'", path.c_str());
		FileStat stat;
		if(fileStats(path, stat) == FS_OK && stat.attr[FileAttribute::Directory]) {
			cwd = path;
			response(250, F("directory changed to /") + cwd); // OK
		} else {
			response(550); // Not found / no access
		}
		break;
	}

	case Command::TYPE:
		response(250); // OK
		break;

	case Command::SIZE: {
		auto path = resolvePath(cwd, data);
		if(!checkFileAccess(cwd, FileOpenFlag::Read)) {
			break;
		}
		FileStat stat;
		if(fileStats(path, stat) == FS_OK) {
			response(213, String(stat.size).c_str()); // File status
		} else {
			response(550); // Not found / no access
		}
		break;
	}

	case Command::DELE: {
		String path = resolvePath(cwd, data);
		if(!checkFileAccess(cwd, FileOpenFlag::Write | FileOpenFlag::Truncate)) {
			break;
		}
		int err = fileDelete(path);
		response((err == FS_OK) ? 250 : 550);
		break;
	}

	case Command::RNFR: {
		renameFrom = data;
		response(350); // Pending further information
		break;
	}

	case Command::RNTO: {
		if(!renameFrom) {
			response(550);
			break;
		}
		String pathFrom = resolvePath(cwd, renameFrom);
		if(!checkFileAccess(pathFrom, FileOpenFlag::Write)) {
			break;
		}
		String pathTo = resolvePath(cwd, data);
		if(!checkFileAccess(pathTo, FileOpenFlag::Write)) {
			break;
		}
		int err = fileRename(pathFrom, pathTo);
		if(err == FS_OK) {
			response(250);
		} else {
			response(550, fileGetErrorString(err));
		}
		break;
	}

	case Command::RETR: {
		String path = resolvePath(cwd, data);
		if(checkFileAccess(path, FileOpenFlag::Read)) {
			createDataConnection(new FtpDataRetrieve(*this, path));
		}
		break;
	}

	case Command::STOR: {
		String path = resolvePath(cwd, data);
		if(checkFileAccess(path, FileOpenFlag::Write)) {
			createDataConnection(new FtpDataStore(*this, path));
		}
		break;
	}

	case Command::LIST:
	case Command::NLST: {
		String path = resolvePath(cwd, data);
		if(checkFileAccess(path, FileOpenFlag::Read)) {
			createDataConnection(new FtpDataFileList(*this, path, command == Command::NLST));
		}
		break;
	}

	case Command::PASV: {
		response(500, F("Passive mode not supported"));
		break;
	}

	case Command::NOOP: {
		response(200);
		break;
	}

	case Command::UNKNOWN:
	default:
		cmd.toUpperCase();
		if(!server.onCommand(cmd, data, *this)) {
			response(502, F("Not supported"));
		}
	}
}

err_t FtpServerConnection::onSent(uint16_t len)
{
	if(dataConnection != nullptr) {
		dataConnection->onReadyToSendData(eTCE_Poll);
	}

	return ERR_OK;
}

void FtpServerConnection::createDataConnection(FtpDataStream* connection)
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

void FtpServerConnection::response(int code, String text, char sep)
{
	String response = String(code, DEC);
	response += sep;
	if(text.length() == 0) {
		if(code >= 200 && code <= 399) { // Just for simplify
			response += _F("OK");
		} else {
			response += _F("FAIL");
		}
	} else {
		response += text;
	}
	response += "\r\n";

	debug_d("> %s", response.c_str());
	writeString(response);
	flush();
}
