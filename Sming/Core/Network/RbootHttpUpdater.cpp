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

void RbootHttpUpdater::addItem(int offset, String firmwareFileUrl, size_t maxSize)
{
	RbootHttpUpdaterItem add;
	add.targetOffset = offset;
	add.url = firmwareFileUrl;
	add.size = maxSize;
	items.add(add);
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

		RbootOutputStream* responseStream = new RbootOutputStream(it.targetOffset, it.size);

		request->setResponseStream(responseStream);

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

	HttpRequest* request = client.getRequest();
	ReadWriteStream* stream = request->getResponseStream();
	debug_d("Finished: URL: %s, Length: %d", request->uri.toString.c_str(), stream->available());

	return 0;
}

int RbootHttpUpdater::updateComplete(HttpConnection& client, bool success)
{
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
	items.clear();
}

void RbootHttpUpdater::applyUpdate()
{
	items.clear();
	if(romSlot == NO_ROM_SWITCH) {
		debug_d("Firmware updated.");
		return;
	}

	// set to boot new rom and then reboot
	debug_d("Firmware updated, rebooting to rom %d...\r\n", romSlot);
	rboot_set_current_rom(romSlot);
	System.restart();
}
