/*
 * rBootHttpUpdate.h
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 *
 *  Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 */

#ifndef SMINGCORE_NETWORK_RBOOTHTTPUPDATE_H_
#define SMINGCORE_NETWORK_RBOOTHTTPUPDATE_H_

#include "Data/Stream/DataSourceStream.h"
#include "HttpClient.h"
#include <rboot-api.h>

#define NO_ROM_SWITCH 0xff

class rBootHttpUpdate;

//typedef void (*otaCallback)(bool result);
typedef Delegate<void(rBootHttpUpdate& client, bool result)> OtaUpdateDelegate;

struct rBootHttpUpdateItem
{
	String url;
	uint32_t targetOffset;
	int size;
};

class rBootItemOutputStream : public ReadWriteStream {
public:
	virtual ~rBootItemOutputStream()
	{
		close();
	}

	virtual bool init();
	virtual bool close();

	void setItem(rBootHttpUpdateItem* item)
	{
		_item = item;
	}

	virtual size_t write(const uint8_t* data, size_t size);

	virtual StreamType getStreamType() const
	{
		return eSST_File;
	}

	virtual size_t readMemoryBlock(char* data, size_t bufSize)
	{
		return 0;
	}

	virtual bool seek(int len)
	{
		return false;
	}

	virtual bool isFinished()
	{
		return true;
	}

protected:
	bool _initialized = false;
	rBootHttpUpdateItem* _item = nullptr;
	rboot_write_status _rBootWriteStatus;
};

class rBootHttpUpdate : protected HttpClient {
public:
	rBootHttpUpdate()
	{}

	virtual ~rBootHttpUpdate()
	{}

	void addItem(int offset, String firmwareFileUrl);
	void start();
	void switchToRom(uint8_t romSlot)
	{
		_romSlot = romSlot;
	}

	void setCallback(OtaUpdateDelegate reqUpdateDelegate)
	{
		setDelegate(reqUpdateDelegate);
	}

	void setDelegate(OtaUpdateDelegate reqUpdateDelegate)
	{
		_updateDelegate = reqUpdateDelegate;
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
		_baseRequest = request;
	}

	// Allow reading items
	rBootHttpUpdateItem getItem(unsigned int index)
	{
		return _items.elementAt(index);
	}

protected:
	void applyUpdate();
	void updateFailed();

	/** @brief override this method to create a custom stream object */
	virtual rBootItemOutputStream* createStream();

	virtual int itemComplete(HttpConnection& client, bool success);
	virtual int updateComplete(HttpConnection& client, bool success);

protected:
	Vector<rBootHttpUpdateItem> _items;
	int _currentItem = 0;
	rboot_write_status _rBootWriteStatus;
	uint8_t _romSlot = NO_ROM_SWITCH;
	OtaUpdateDelegate _updateDelegate = nullptr;
	HttpRequest* _baseRequest = nullptr;
};

#endif /* SMINGCORE_NETWORK_RBOOTHTTPUPDATE_H_ */
