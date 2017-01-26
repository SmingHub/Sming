/*
 * rBootHttpUpdate.h
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 */

#ifndef SMINGCORE_NETWORK_RBOOTHTTPUPDATE_H_
#define SMINGCORE_NETWORK_RBOOTHTTPUPDATE_H_

#include "HttpClient.h"
#include <Timer.h>

#include <rboot-api.h>

#define NO_ROM_SWITCH 0xff

//typedef void (*otaCallback)(bool result);
typedef Delegate<void(bool result)> otaUpdateDelegate;

struct rBootHttpUpdateItem {
	String url;
	uint32_t targetOffset;
	int size;
};

class rBootHttpUpdate: private HttpClient {

public:
	rBootHttpUpdate();
	virtual ~rBootHttpUpdate();
	void addItem(int offset, String firmwareFileUrl);
	void start();
	void switchToRom(uint8 romSlot);
	void setCallback(otaUpdateDelegate reqUpdateDelegate);
	void setDelegate(otaUpdateDelegate reqUpdateDelegate);

	using HttpClient::addSslOptions;
	using HttpClient::setSslFingerprint;
	using HttpClient::setSslClientKeyCert;
	using HttpClient::freeSslClientKeyCert;
#ifdef ENABLE_SSL
	using HttpClient::getSsl;
#endif

protected:
	void onTimer();
	virtual void writeRawData(pbuf* buf, int startPos);
	void applyUpdate();
	void updateFailed();
	void onItemDownloadCompleted(HttpClient& client, bool successful);

protected:
	Vector<rBootHttpUpdateItem> items;
	Timer timer;
	int currentItem;
	rboot_write_status rBootWriteStatus;
	uint8 romSlot;
	otaUpdateDelegate updateDelegate;

    virtual void writeInit();
    virtual bool writeFlash(const u8 *data, u16 size);
    virtual bool writeEnd();
};

#endif /* SMINGCORE_NETWORK_RBOOTHTTPUPDATE_H_ */
