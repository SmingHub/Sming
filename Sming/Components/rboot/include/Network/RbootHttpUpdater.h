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

/**
 * @brief Magic value for ROM slot indicating slot won't change after successful OTA
 */
constexpr uint8_t NO_ROM_SWITCH{0xff};

class RbootHttpUpdater;

using OtaUpdateDelegate = Delegate<void(RbootHttpUpdater& client, bool result)>;

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

	/**
	 * @brief Add an item to update
	 * @param offset
	 * @param firmwareFileUrl
	 * @param maxSize
	 * @retval bool
	 * @note Use the `Partition` overload where possible
	 */
	bool addItem(uint32_t offset, const String& firmwareFileUrl, size_t maxSize = 0);

	/**
	 * @brief Add an item to update
	 * @param firmwareFileUrl
	 * @param partition Target partition to write
	 * @retval bool
	 */
	bool addItem(const String& firmwareFileUrl, Partition partition)
	{
		return addItem(partition.address(), firmwareFileUrl, partition.size());
	}

	/**
	 * @brief Add an item to update use a pre-constructed stream
	 * @param firmwareFileUrl
	 * @param stream
	 * @retval bool
	 */
	bool addItem(const String& firmwareFileUrl, RbootOutputStream* stream);

	void start();

	/**
	 * @brief On completion, switch to the given ROM slot
	 * @param romSlot specify NO_ROM_SWITCH (the default) to cancel any previously set switch
	 */
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

	/**
	 *  @brief Sets the base request that can be used to pass
	 * 
	 * 		- default request parameters, like request headers...
	 * 		- default SSL options
	 * 		- default SSL fingeprints
	 * 		- default SSL client certificates
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
	OtaUpdateDelegate updateDelegate;
	HttpRequest* baseRequest{nullptr};
	uint8_t romSlot{NO_ROM_SWITCH};
	uint8_t currentItem{0};
	rboot_write_status rbootWriteStatus{};

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

/** @deprecated Use `RbootHttpUpdater` */
typedef RbootHttpUpdater rBootHttpUpdate SMING_DEPRECATED;
