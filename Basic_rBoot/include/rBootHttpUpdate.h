/*
 * rBootHttpUpdate.h
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 */

#ifndef _rBootHttpUpdate_H_
#define _rBootHttpUpdate_H_

#include <Network/HttpClient.h>
#include <Timer.h>

#include "rboot-api.h"

#define NO_ROM_SWITCH 0xff

typedef void (*otaCallback)(bool result);

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
	void setCallback(otaCallback callback);

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
	otaCallback callback;
};

#endif /* _rBootHttpUpdate_H_ */
