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

class FtpDataFileList : public FtpDataStream
{
public:
	explicit FtpDataFileList(FtpServerConnection& connection) : FtpDataStream(connection)
	{
	}

	void transferData(TcpConnectionEvent sourceEvent) override
	{
		if(completed) {
			return;
		}
		Vector<String> list = fileList();
		debug_d("send file list: %d", list.count());
		for(unsigned i = 0; i < list.count(); i++) {
			String s = F("01-01-15  01:00AM               ");
			s += fileGetSize(list[i]);
			s += ' ';
			s += list[i];
			s += "\r\n";
			writeString(s);
		}
		completed = true;
		finishTransfer();
	}
};
