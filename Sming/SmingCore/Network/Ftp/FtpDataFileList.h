/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpDataFileList.h
 *
 ****/

#ifndef _SMING_CORE_NETWORK_FTP_FTP_DATA_FILE_LIST_H_
#define _SMING_CORE_NETWORK_FTP_FTP_DATA_FILE_LIST_H_

#include "FtpDataStream.h"
#include "FileSystem.h"

class FtpDataFileList : public FtpDataStream
{
public:
	explicit FtpDataFileList(FtpServerConnection* connection) : FtpDataStream(connection)
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
			writeString("01-01-15  01:00AM               " + String(fileGetSize(list[i])) + " " + list[i] + "\r\n");
		}
		completed = true;
		finishTransfer();
	}
};

#endif /* _SMING_CORE_NETWORK_FTP_FTP_DATA_FILE_LIST_H_ */
