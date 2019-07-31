/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * rBootHttpUpdate.h
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 *
 *  Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "Data/Stream/DataSourceStream.h"
#include "Network/HttpClient.h"
#include <rboot-api.h>

#define NO_ROM_SWITCH 0xff

class rBootHttpUpdate;

typedef Delegate<void(rBootHttpUpdate& client, bool result)> OtaUpdateDelegate;

struct rBootHttpUpdateItem {
	String url;
	uint32_t targetOffset;
	int size;
};

class rBootItemOutputStream : public ReadWriteStream
{
public:
	virtual ~rBootItemOutputStream()
	{
		close();
	}

	void setItem(rBootHttpUpdateItem* item)
	{
		this->item = item;
	}

	virtual bool init();

	size_t write(const uint8_t* data, size_t size) override;

	StreamType getStreamType() const override
	{
		return eSST_File;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		return 0;
	}

	bool seek(int len) override
	{
		return false;
	}

	bool isFinished() override
	{
		return true;
	}

	virtual bool close();

protected:
	bool initialized = false;
	rBootHttpUpdateItem* item = nullptr;
	rboot_write_status rBootWriteStatus;
};

class rBootHttpUpdate : protected HttpClient
{
public:
	void addItem(int offset, String firmwareFileUrl);
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
	 * @param HttpRequest *
	 */
	void setBaseRequest(HttpRequest* request)
	{
		baseRequest = request;
	}

	// Allow reading items
	rBootHttpUpdateItem getItem(unsigned int index)
	{
		return items.elementAt(index);
	}

protected:
	void applyUpdate();
	void updateFailed();

	virtual rBootItemOutputStream* getStream()
	{
		return new rBootItemOutputStream();
	}

	virtual int itemComplete(HttpConnection& client, bool success);
	virtual int updateComplete(HttpConnection& client, bool success);

protected:
	Vector<rBootHttpUpdateItem> items;
	int currentItem = 0;
	rboot_write_status rBootWriteStatus;
	uint8_t romSlot = NO_ROM_SWITCH;
	OtaUpdateDelegate updateDelegate = nullptr;

	HttpRequest* baseRequest = nullptr;
};
