/*
 * rBootHttpUpdate.cpp
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 *
 *  Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 */

#include "rBootHttpUpdate.h"
#include "Platform/System.h"
#include "URL.h"
#include "Platform/WDT.h"

/* rBootItemOutputStream */

bool rBootItemOutputStream::init()
{
	if (!_item) {
		debug_e("rBootItemOutputStream: Item must be set!");
		return false;
	}

	_rBootWriteStatus = rboot_write_init(_item->targetOffset);
	_initialized = true;

	return true;
}

size_t rBootItemOutputStream::write(const uint8_t* data, size_t size)
{
	if (!_initialized && size > 0) {
		if (!init())
			return 0;

		_initialized = true;
	}

	if (!rboot_write_flash(&_rBootWriteStatus, (uint8_t*)data, size)) {
		debug_e("rboot_write_flash: Failed. Size: %d", size);
		return 0;
	}

	_item->size += size;

	debug_d("rboot_write_flash: item.size: %d", _item->size);

	return size;
}

bool rBootItemOutputStream::close()
{
	return rboot_write_end(&_rBootWriteStatus);
}

/* rBootHttpUpdate */

void rBootHttpUpdate::addItem(int offset, String firmwareFileUrl)
{
	rBootHttpUpdateItem add;
	add.targetOffset = offset;
	add.url = firmwareFileUrl;
	add.size = 0;
	_items.add(add);
}

void rBootHttpUpdate::start()
{
	for (unsigned i = 0; i < _items.count(); i++) {
		rBootHttpUpdateItem& it = _items[i];
		debug_d("Download file:\r\n    (%d) %s -> %X", _currentItem, it.url.c_str(), it.targetOffset);

		HttpRequest* request;
		if (_baseRequest) {
			request = _baseRequest->clone();
			request->setURL(URL(it.url));
		}
		else
			request = new HttpRequest(URL(it.url));

		request->setMethod(HTTP_GET);

		auto responseStream = createStream();
		responseStream->setItem(&it);

		request->setResponseStream(responseStream);

		if (i == _items.count() - 1)
			request->onRequestComplete(RequestCompletedDelegate(&rBootHttpUpdate::updateComplete, this));
		else
			request->onRequestComplete(RequestCompletedDelegate(&rBootHttpUpdate::itemComplete, this));

		if (!send(request)) {
			debug_e("ERROR: Rejected sending new request.");
			break;
		}
	}
}

rBootItemOutputStream* rBootHttpUpdate::createStream()
{
	return new rBootItemOutputStream();
}

int rBootHttpUpdate::itemComplete(HttpConnection& client, bool success)
{
	if (!success) {
		updateFailed();
		return -1;
	}

	return 0;
}

int rBootHttpUpdate::updateComplete(HttpConnection& client, bool success)
{
	debug_d("\r\nFirmware download finished!");
	for (unsigned i = 0; i < _items.count(); i++)
		debug_d(" - item: %d, addr: %X, len: %d bytes", i, _items[i].targetOffset, _items[i].size);

	if (!success) {
		updateFailed();
		return -1;
	}

	if (_updateDelegate)
		_updateDelegate(*this, true);

	applyUpdate();

	return 0;
}

void rBootHttpUpdate::updateFailed()
{
	debug_e("\r\nFirmware download failed..");
	if (_updateDelegate)
		_updateDelegate(*this, false);
	_items.clear();
}

void rBootHttpUpdate::applyUpdate()
{
	_items.clear();
	if (_romSlot == NO_ROM_SWITCH) {
		debug_d("Firmware updated.");
		return;
	}

	// set to boot new rom and then reboot
	debug_d("Firmware updated, rebooting to rom %d...\r\n", _romSlot);
	rboot_set_current_rom(_romSlot);
	System.restart();
}
