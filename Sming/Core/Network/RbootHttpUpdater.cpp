/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RbootHttpUpdater.cpp
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 *
 *  Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 */

#include "RbootHttpUpdater.h"

#include <Platform/System.h>
#include <Network/Url.h>
#include <Platform/WDT.h>

bool RbootHttpUpdater::addItem(int offset, const String& firmwareFileUrl, size_t maxSize)
{
	RbootHttpUpdaterItem add;
	add.targetOffset = offset;
	add.url = firmwareFileUrl;
	add.size = maxSize;
	add.stream = nullptr;
	return items.add(add);
}

bool RbootHttpUpdater::addItem(const String& firmwareFileUrl, RbootOutputStream* stream)
{
	if(stream == nullptr) {
		return false;
	}

	RbootHttpUpdaterItem add;
	memset(&add, 0, sizeof(RbootHttpUpdaterItem));
	add.targetOffset = stream->getStartAddress();
	add.url = firmwareFileUrl;
	add.size = stream->getMaxLength();
	add.stream = stream;

	return items.add(add);
}

void RbootHttpUpdater::start()
{
	for(unsigned i = 0; i < items.count(); i++) {
		RbootHttpUpdaterItem& it = items[i];
		debug_d("Download file:\r\n    (%d) %s -> %X", currentItem, it.url.c_str(), it.targetOffset);

		HttpRequest* request;
		if(baseRequest != nullptr) {
			request = baseRequest->clone();
			request->setURL(it.url);
		} else {
			request = new HttpRequest(it.url);
		}

		request->setMethod(HTTP_GET);

		if(it.stream == nullptr) {
			it.stream = new RbootOutputStream(it.targetOffset, it.size);
		}

		request->setResponseStream(it.stream);

		if(i == items.count() - 1) {
			request->onRequestComplete(RequestCompletedDelegate(&RbootHttpUpdater::updateComplete, this));
		} else {
			request->onRequestComplete(RequestCompletedDelegate(&RbootHttpUpdater::itemComplete, this));
		}

		if(!send(request)) {
			debug_e("ERROR: Rejected sending new request.");
			break;
		}
	}
}

int RbootHttpUpdater::itemComplete(HttpConnection& client, bool success)
{
	if(!success) {
		updateFailed();
		return -1;
	}

	RbootHttpUpdaterItem& it = items[currentItem];
	debug_d("Finished: URL: %s, Offset: %d, Length: %d", it.url.c_str(), it.stream->getStartAddress(),
			it.stream->available());

	it.stream = nullptr; // the actual deletion will happen outside of this class
	currentItem++;

	return 0;
}

int RbootHttpUpdater::updateComplete(HttpConnection& client, bool success)
{
	auto hasError = itemComplete(client, success);
	if(hasError) {
		return hasError;
	}

	debug_d("\r\nFirmware download finished!");
	for(unsigned i = 0; i < items.count(); i++) {
		debug_d(" - item: %d, addr: %X, url: %s", i, items[i].targetOffset, items[i].url.c_str());
	}

	if(!success) {
		updateFailed();
		return -1;
	}

	if(updateDelegate) {
		updateDelegate(*this, true);
	}

	applyUpdate();

	return 0;
}

void RbootHttpUpdater::updateFailed()
{
	debug_e("\r\nFirmware download failed..");
	if(updateDelegate) {
		updateDelegate(*this, false);
	}
	cleanup();
}

void RbootHttpUpdater::applyUpdate()
{
	cleanup();
	if(romSlot == NO_ROM_SWITCH) {
		debug_d("Firmware updated.");
		return;
	}

	// set to boot new rom and then reboot
	debug_d("Firmware updated, rebooting to rom %d...\r\n", romSlot);
	rboot_set_current_rom(romSlot);
	System.restart();
}
