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

class rBootHttpUpdate;

//typedef void (*otaCallback)(bool result);
typedef Delegate<void(rBootHttpUpdate& client, bool result)> OtaUpdateDelegate;

struct rBootHttpUpdateItem {
	String url;
	uint32_t targetOffset;
	int size;
};

class rBootHttpUpdate: protected HttpClient {

public:
	rBootHttpUpdate();
	virtual ~rBootHttpUpdate();
	void addItem(int offset, String firmwareFileUrl);
	void start();
	void switchToRom(uint8 romSlot);
	void setCallback(OtaUpdateDelegate reqUpdateDelegate);
	void setDelegate(OtaUpdateDelegate reqUpdateDelegate);


	// Expose request and response header information
	using HttpClient::setRequestHeader;
	using HttpClient::hasRequestHeader;
	using HttpClient::getResponseHeader;

	// Allow reading items
	rBootHttpUpdateItem getItem(unsigned int index);

#ifdef ENABLE_SSL
	using HttpClient::addSslOptions;
	using HttpClient::setSslFingerprint;
	using HttpClient::pinCertificate;
	using HttpClient::setSslClientKeyCert;
	using HttpClient::freeSslClientKeyCert;
	using HttpClient::getSsl;
#endif

protected:
	void onTimer();
	virtual err_t onResponseBody(const char *at, size_t length);
	void applyUpdate();
	void updateFailed();

protected:
	Vector<rBootHttpUpdateItem> items;
	Timer timer;
	int currentItem;
	rboot_write_status rBootWriteStatus;
	uint8 romSlot;
	OtaUpdateDelegate updateDelegate;

	virtual void writeInit();
	virtual bool writeFlash(const u8 *data, u16 size);
	virtual bool writeEnd();
};

#endif /* SMINGCORE_NETWORK_RBOOTHTTPUPDATE_H_ */
