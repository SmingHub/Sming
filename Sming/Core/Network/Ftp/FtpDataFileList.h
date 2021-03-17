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
#include "FileSystem.h"
#include <DateTime.h>
#include <SystemClock.h>

class FtpDataFileList : public FtpDataStream
{
public:
	explicit FtpDataFileList(FtpServerConnection& connection, const String& path, bool namesOnly = false)
		: FtpDataStream(connection), namesOnly(namesOnly)
	{
		auto& stat = const_cast<FileStat&>(dir.stat());
		if(fileStats(path, stat) == FS_OK && !stat.attr[FileAttribute::Directory]) {
			isFile = true;
		} else {
			isFile = false;
			dir.open(path);
		}

		year = DateTime(SystemClock.now()).Year;
	}

	void transferData(TcpConnectionEvent sourceEvent) override
	{
		if(completed) {
			return;
		}

		if(isFile) {
			writeStat(dir.stat());
			completed = true;
			return;
		}

		if(dir.next()) {
			writeStat(dir.stat());
			return;
		}

		debug_d("sent file list: %u", dir.count());
		completed = true;
		finishTransfer();
	}

	void writeStat(const FileStat& stat)
	{
		if(!namesOnly) {
			auto& user = control.getUser();
			DateTime dt{stat.mtime};
			String timestr = dt.format((dt.Year == year) ? _F("%b %e %H:%M") : _F("%b %e  %Y"));
			char buf[128];
			PSTR_ARRAY(fmt, "--- %-6u %s ");
			m_snprintf(buf, sizeof(buf), fmt, stat.size, timestr.c_str());
			buf[0] = stat.attr[FileAttribute::Directory] ? 'd' : '-';
			buf[1] = (user.role >= stat.acl.readAccess) ? 'r' : '-';
			buf[2] = stat.attr[FileAttribute::ReadOnly] && (user.role >= stat.acl.writeAccess) ? 'w' : '-';
			writeString(buf);
		}
		write(stat.name, stat.name.length);
		write("\r\n", 2);
	}

private:
	uint16_t year;
	Directory dir;
	bool namesOnly;
	bool isFile;
};
