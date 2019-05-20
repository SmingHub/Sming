/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * rBootHttpUpdate.cpp
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 *
 *  Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 */

#include "rBootHttpUpdate.h"
#include "Platform/System.h"
#include "Network/Url.h"
#include "Platform/WDT.h"

/* rBootItemOutputStream */

bool rBootItemOutputStream::init()
{
	if(item == nullptr) {
		debug_e("rBootItemOutputStream: Item must be set!");
		return false;
	}

	rBootWriteStatus = rboot_write_init(this->item->targetOffset);
	initialized = true;

	return true;
}

size_t rBootItemOutputStream::write(const uint8_t* data, size_t size)
{
	if(!initialized && size > 0) {
		if(!init()) { // unable to initialize
			return -1;
		}

		initialized = true;
	}

	if(!rboot_write_flash(&rBootWriteStatus, (uint8_t*)data, size)) {
		debug_e("rboot_write_flash: Failed. Size: %d", size);
		return -1;
	}

	item->size += size;

	debug_d("rboot_write_flash: item.size: %d", item->size);

	return size;
}

bool rBootItemOutputStream::close()
{
	return rboot_write_end(&rBootWriteStatus);
}

/* rBootHttpUpdate */

void rBootHttpUpdate::addItem(int offset, String firmwareFileUrl)
{
	rBootHttpUpdateItem add;
	add.targetOffset = offset;
	add.url = firmwareFileUrl;
	add.size = 0;
	items.add(add);
}

void rBootHttpUpdate::start()
{
	for(unsigned i = 0; i < items.count(); i++) {
		rBootHttpUpdateItem& it = items[i];
		debug_d("Download file:\r\n    (%d) %s -> %X", currentItem, it.url.c_str(), it.targetOffset);

		HttpRequest* request;
		if(baseRequest != nullptr) {
			request = baseRequest->clone();
			request->setURL(it.url);
		} else {
			request = new HttpRequest(it.url);
		}

		request->setMethod(HTTP_GET);

		rBootItemOutputStream* responseStream = getStream();
		responseStream->setItem(&it);

		request->setResponseStream(responseStream);

		if(i == items.count() - 1) {
			request->onRequestComplete(RequestCompletedDelegate(&rBootHttpUpdate::updateComplete, this));
		} else {
			request->onRequestComplete(RequestCompletedDelegate(&rBootHttpUpdate::itemComplete, this));
		}

		if(!send(request)) {
			debug_e("ERROR: Rejected sending new request.");
			break;
		}
	}
}

int rBootHttpUpdate::itemComplete(HttpConnection& client, bool success)
{
	if(!success) {
		updateFailed();
		return -1;
	}

	return 0;
}

int rBootHttpUpdate::updateComplete(HttpConnection& client, bool success)
{
	debug_d("\r\nFirmware download finished!");
	for(unsigned i = 0; i < items.count(); i++) {
		debug_d(" - item: %d, addr: %X, len: %d bytes", i, items[i].targetOffset, items[i].size);
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

void rBootHttpUpdate::updateFailed()
{
	debug_e("\r\nFirmware download failed..");
	if(updateDelegate) {
		updateDelegate(*this, false);
	}
	items.clear();
}

void rBootHttpUpdate::applyUpdate()
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
