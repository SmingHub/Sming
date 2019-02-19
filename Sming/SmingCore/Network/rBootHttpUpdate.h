/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
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

#ifndef _SMING_CORE_NETWORK_RBOOT_HTTP_UPDATE_H_
#define _SMING_CORE_NETWORK_RBOOT_HTTP_UPDATE_H_

#include "Data/Stream/DataSourceStream.h"
#include "HttpClient.h"
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

class rBootItemOutputStream : public ReadWriteStream
{
public:
	~rBootItemOutputStream();

	void setItem(rBootHttpUpdateItem* item);
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
	bool initilized = false;
	rBootHttpUpdateItem* item = nullptr;
	rboot_write_status rBootWriteStatus;
};

class rBootHttpUpdate : protected HttpClient
{
public:
	void addItem(int offset, String firmwareFileUrl);
	void start();
	void switchToRom(uint8_t romSlot);
	void setCallback(OtaUpdateDelegate reqUpdateDelegate);
	void setDelegate(OtaUpdateDelegate reqUpdateDelegate);

	/* Sets the base request that can be used to pass
	 * - default request parameters, like request headers...
	 * - default SSL options
	 * - default SSL fingeprints
	 * - default SSL client certificates
	 *
	 * @param HttpRequest *
	 */
	void setBaseRequest(HttpRequest* request);

	// Allow reading items
	rBootHttpUpdateItem getItem(unsigned int index);

protected:
	void applyUpdate();
	void updateFailed();

	virtual rBootItemOutputStream* getStream();
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

#endif /* _SMING_CORE_NETWORK_RBOOT_HTTP_UPDATE_H_ */
