/*
 * HttpFirmwareUpdate.h
 *
 *  Created on: 26 ���. 2015 �.
 *      Author: Anakod
 */

#ifndef SMINGCORE_NETWORK_HTTPFIRMWAREUPDATE_H_
#define SMINGCORE_NETWORK_HTTPFIRMWAREUPDATE_H_

#include "HttpClient.h"
#include <Timer.h>

struct HttpFirmwareUpdateItem
{
	String url;
	uint32_t targetOffset;
	uint32_t flash;
	int size;
};

class HttpFirmwareUpdate: private HttpClient
{
public:
	HttpFirmwareUpdate();
	virtual ~HttpFirmwareUpdate();

	void addItem(int offset, String firmwareFileUrl);
	void start();

	__forceinline bool isProcessing()  { return TcpClient::isProcessing(); }
	__forceinline TcpClientState getConnectionState() { return TcpClient::getConnectionState(); }

protected:
	void onTimer();
	virtual err_t onResponseBody(const char *at, size_t length);
	uint32_t writeFlash(const char* data, uint32_t pos, int size);
	void applyUpdate();
	void updateFailed();

protected:
	Vector<HttpFirmwareUpdateItem> items;
	Timer timer;
	int currentItem;
	uint32_t pos;
};

#endif /* SMINGCORE_NETWORK_HTTPFIRMWAREUPDATE_H_ */
