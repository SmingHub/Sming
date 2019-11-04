/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpDataRetrieve.h
 *
 ****/

#pragma once

#include "FtpDataStream.h"
#include "FileSystem.h"

class FtpDataRetrieve : public FtpDataStream
{
public:
	FtpDataRetrieve(FtpServerConnection* connection, const String& fileName)
		: FtpDataStream(connection), file(fileOpen(fileName, eFO_ReadOnly))
	{
	}

	~FtpDataRetrieve()
	{
		fileClose(file);
	}

	void transferData(TcpConnectionEvent sourceEvent) override
	{
		if(completed) {
			return;
		}
		char buf[1024];
		int len = fileRead(file, buf, sizeof(buf));
		write(buf, len, TCP_WRITE_FLAG_COPY);
		if(fileIsEOF(file)) {
			completed = true;
			finishTransfer();
		}
	}

private:
	file_t file;
};
