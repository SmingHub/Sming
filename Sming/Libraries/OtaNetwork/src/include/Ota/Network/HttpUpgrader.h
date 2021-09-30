/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpUpgrader.h
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 *
 *  Modified: 2017, 2021 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/HttpClient.h>
#include <Ota/UpgradeOutputStream.h>

namespace Ota
{
namespace Network
{
/**
 * @brief Magic value for ROM slot indicating slot won't change after successful OTA
 */
constexpr uint8_t NO_ROM_SWITCH{0xff};

class HttpUpgrader : protected HttpClient
{
public:
	using CompletedDelegate = Delegate<void(HttpUpgrader& client, bool result)>;
	using Partition = Storage::Partition;

	struct Item {
		String url;
		Partition partition;			  // << partition to write the data to
		size_t size{0};					  // << actual size of written bytes
		ReadWriteStream* stream{nullptr}; // (optional) output stream to use.

		Item(String url, Partition partition, ReadWriteStream* stream) : url(url), partition(partition), stream(stream)
		{
		}

		~Item()
		{
			delete stream;
		}

		ReadWriteStream* getStream()
		{
			if(stream == nullptr) {
				stream = new Ota::UpgradeOutputStream(partition);
			}
			return stream;
		}
	};

	class ItemList : public Vector<Item>
	{
	public:
		bool addNew(Item* it)
		{
			if(addElement(it)) {
				return true;
			}
			delete it;
			return false;
		}
	};

	/**
	 * @brief Add an item to update
	 * @param firmwareFileUrl
	 * @param partition Target partition to write
	 * @param stream
	 *
	 * @retval bool
	 */
	bool addItem(const String& firmwareFileUrl, Partition partition, ReadWriteStream* stream = nullptr)
	{
		return items.addNew(new Item{firmwareFileUrl, partition, stream});
	}

	void start();

	/**
	 * @brief On completion, switch to the given ROM slot
	 * @param romSlot specify NO_ROM_SWITCH (the default) to cancel any previously set switch
	 */
	void switchToRom(uint8_t romSlot)
	{
		this->romSlot = romSlot;
	}

	void setCallback(CompletedDelegate reqUpdateDelegate)
	{
		setDelegate(reqUpdateDelegate);
	}

	void setDelegate(CompletedDelegate reqUpdateDelegate)
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

	/**
	 * @brief Allow read access to item list
	 */
	const ItemList& getItems() const
	{
		return items;
	}

protected:
	void applyUpdate();
	void updateFailed();

	virtual int itemComplete(HttpConnection& client, bool success);
	virtual int updateComplete(HttpConnection& client, bool success);

protected:
	ItemList items;
	CompletedDelegate updateDelegate;
	HttpRequest* baseRequest{nullptr};
	uint8_t romSlot{NO_ROM_SWITCH};
	uint8_t currentItem{0};
};

} // namespace Network

} // namespace Ota
