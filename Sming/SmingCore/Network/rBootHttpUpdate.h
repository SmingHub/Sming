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

protected:
	void onTimer();
	virtual void writeRawData(pbuf* buf, int startPos);
	void applyUpdate();
	void updateFailed();

protected:
	Vector<rBootHttpUpdateItem> items;
	Timer timer;
	int currentItem;
	rboot_write_status rBootWriteStatus;
	uint8 romSlot;
	otaUpdateDelegate updateDelegate;
};

#endif /* SMINGCORE_NETWORK_RBOOTHTTPUPDATE_H_ */
