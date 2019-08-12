/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RbootHttpUpdater.h
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 *
 *  Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/HttpClient.h>
#include "../Data/Stream/RbootOutputStream.h"

#define NO_ROM_SWITCH 0xff

class RbootHttpUpdater;

typedef Delegate<void(RbootHttpUpdater& client, bool result)> OtaUpdateDelegate;

struct RbootHttpUpdaterItem {
	String url;
	uint32_t targetOffset;
	size_t size;						 // << max allowed size
	RbootOutputStream* stream = nullptr; // (optional) output stream to use.
};

class RbootHttpUpdater : protected HttpClient
{
public:
	virtual ~RbootHttpUpdater()
	{
		cleanup();
	}

	bool addItem(int offset, const String& firmwareFileUrl, size_t maxSize = 0);
	bool addItem(const String& firmwareFileUrl, RbootOutputStream* stream = nullptr);

	void start();

	void switchToRom(uint8_t romSlot)
	{
		this->romSlot = romSlot;
	}

	void setCallback(OtaUpdateDelegate reqUpdateDelegate)
	{
		setDelegate(reqUpdateDelegate);
	}

	void setDelegate(OtaUpdateDelegate reqUpdateDelegate)
	{
		this->updateDelegate = reqUpdateDelegate;
	}

	/* Sets the base request that can be used to pass
	 * - default request parameters, like request headers...
	 * - default SSL options
	 * - default SSL fingeprints
	 * - default SSL client certificates
	 *
	 * @param request
	 */
	void setBaseRequest(HttpRequest* request)
	{
		baseRequest = request;
	}

	// Allow reading items
	RbootHttpUpdaterItem getItem(unsigned int index)
	{
		return items.elementAt(index);
	}

protected:
	void applyUpdate();
	void updateFailed();

	virtual int itemComplete(HttpConnection& client, bool success);
	virtual int updateComplete(HttpConnection& client, bool success);

protected:
	Vector<RbootHttpUpdaterItem> items;
	int currentItem = 0;
	rboot_write_status rbootWriteStatus;
	uint8_t romSlot = NO_ROM_SWITCH;
	OtaUpdateDelegate updateDelegate = nullptr;

	HttpRequest* baseRequest = nullptr;

private:
	void cleanup()
	{
		for(unsigned i = 0; i < items.count(); i++) {
			delete items[i].stream;
			items[i].stream = nullptr;
		}
		items.clear();
	}
};

/** @deprecated Use `RbootOutputStream` */
typedef RbootHttpUpdater rBootHttpUpdate SMING_DEPRECATED;
