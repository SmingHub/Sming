/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpDataStore.h
 *
 ****/

#ifndef _SMING_CORE_NETWORK_FTP_FTP_DATA_STORE_H_
#define _SMING_CORE_NETWORK_FTP_FTP_DATA_STORE_H_

#include "FtpDataStream.h"
#include "FileSystem.h"

class FtpDataStore : public FtpDataStream
{
public:
	FtpDataStore(FtpServerConnection* connection, const String& fileName)
		: FtpDataStream(connection), file(fileOpen(fileName, eFO_WriteOnly | eFO_CreateNewAlways))
	{
	}

	~FtpDataStore()
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

#endif /* _SMING_CORE_NETWORK_FTP_FTP_DATA_STORE_H_ */
