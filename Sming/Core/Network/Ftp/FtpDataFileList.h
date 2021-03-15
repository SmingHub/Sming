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

class FtpDataFileList : public FtpDataStream
{
public:
	explicit FtpDataFileList(FtpServerConnection& connection, const String& path = nullptr) : FtpDataStream(connection)
	{
		auto& stat = const_cast<FileStat&>(dir.stat());
		if(fileStats(path, stat) == FS_OK && !stat.attr[FileAttribute::Directory]) {
			isFile = true;
		} else {
			isFile = false;
			dir.open(path);
		}
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
		DateTime dt{stat.mtime};
		char buf[64];
		int n = m_snprintf(buf, sizeof(buf), _F("%02u-%02u-%02u  %02u:%02u%cM               %-6u "), dt.Day, dt.Month,
						   dt.Year - 2000, (dt.Hour % 12) ?: 12, dt.Minute, dt.Hour >= 12 ? 'P' : 'A', stat.size);
		if(n <= 0) {
			writeString(F("ERROR"));
		} else {
			write(buf, n);
		}
		write(stat.name, stat.name.length);
		write("\r\n", 2);
	}

private:
	Directory dir;
	bool isFile;
};
