/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpDataFileList.h
 *
 ****/

#pragma once

#include "FtpDataStream.h"
#include <DateTime.h>
#include <SystemClock.h>

class FtpDataFileList : public FtpDataStream
{
public:
	explicit FtpDataFileList(FtpServerConnection& connection, const String& path, bool namesOnly = false)
		: FtpDataStream(connection), dir(connection.getFileSystem()), namesOnly(namesOnly)
	{
		auto fs = dir.getFileSystem();
		assert(fs != nullptr);
		auto& stat = const_cast<FileStat&>(dir.stat());
		auto err = fs->stat(path, stat);
		if(err != FS_OK) {
			completed = true;
			return;
		}

		if(stat.isDir()) {
			dir.open(path);
			statValid = dir.next();
		} else {
			statValid = true;
		}

		if(SystemClock.isSet()) {
			year = DateTime(SystemClock.now()).Year;
		} else {
			debug_w("[FTP] Warning: system clock hasn't been set!");
		}
	}

	void transferData(TcpConnectionEvent sourceEvent) override
	{
		if(completed) {
			return;
		}

		String line;
		while(statValid) {
			getStatLine(dir.stat(), line);
			line += "\r\n";
			if(line.length() > getAvailableWriteSize()) {
				return;
			}
			int written = writeString(line);
			if(written < 0) {
				return;
			}
			statValid = dir.next();
		}

		debug_d("sent file list: %u", dir.count());
		completed = true;
		finishTransfer();
	}

	void getStatLine(const FileStat& stat, String& line)
	{
		line.setLength(0);
		if(!namesOnly) {
			auto& user = control.getUser();
			DateTime dt{stat.mtime};
			String timestr = dt.format((dt.Year == year) ? _F("%b %e %H:%M") : _F("%b %e  %Y"));
			char buf[128];
			PSTR_ARRAY(fmt, "--- %-6u %s ");
			m_snprintf(buf, sizeof(buf), fmt, stat.size, timestr.c_str());
			buf[0] = stat.isDir() ? 'd' : '-';
			buf[1] = (user.role >= stat.acl.readAccess) ? 'r' : '-';
			buf[2] = stat.attr[FileAttribute::ReadOnly] && (user.role >= stat.acl.writeAccess) ? 'w' : '-';
			line = buf;
		}
		line.concat(stat.name, stat.name.length);
		if(stat.isDir()) {
			line += '/';
		}
	}

private:
	IFS::Directory dir;
	uint16_t year{0};
	bool namesOnly;
	bool statValid{false};
};
