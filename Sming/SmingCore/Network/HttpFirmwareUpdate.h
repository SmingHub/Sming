/*
 * HttpFirmwareUpdate.h
 *
 *  Created on: 26 апр. 2015 г.
 *      Author: Anakod
 */

#ifndef SMINGCORE_NETWORK_HTTPFIRMWAREUPDATE_H_
#define SMINGCORE_NETWORK_HTTPFIRMWAREUPDATE_H_

#include "HttpClient.h"

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

protected:
	static void staticOnTimer(void* ptrSelf);
	void onTimer();
	virtual void writeRawData(pbuf* buf, int startPos);
	uint32_t writeFlash(char* data, uint32_t pos, int size);
	void applyUpdate();

protected:
	Vector<HttpFirmwareUpdateItem> items;
	ETSTimer timer;
	int currentItem;
	uint32_t pos;
};

#endif /* SMINGCORE_NETWORK_HTTPFIRMWAREUPDATE_H_ */
