/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpUpdater.cpp
 *
 */

#include "include/Ota/Network/HttpUpgrader.h"
#include <Ota/Manager.h>

namespace Ota
{
namespace Network
{
void HttpUpgrader::start()
{
	fetchNextItem();
}

void HttpUpgrader::fetchNextItem()
{
	if(currentItem >= items.count()) {
		return;
	}

	auto& it = items[currentItem];
	debug_d("Download file:\r\n"
			"    (%u) %s -> %s @ 0x%X",
			currentItem, it.url.c_str(), it.partition.name().c_str(), it.partition.address());

	HttpRequest* request;
	if(baseRequest != nullptr) {
		request = baseRequest->clone();
		request->setURL(it.url);
	} else {
		request = new HttpRequest(it.url);
	}

	request->setMethod(HTTP_GET);
	request->setResponseStream(it.getStream());

	request->onRequestComplete(RequestCompletedDelegate(&HttpUpgrader::itemComplete, this));

	if(!send(request)) {
		debug_e("ERROR: Rejected sending new request.");
		it.stream.release();
		downloadFailed();
	}
}

int HttpUpgrader::itemComplete(HttpConnection&, bool success)
{
	auto& it = items[currentItem];

	if(!success) {
		it.stream.release();
		downloadFailed();
		return -1;
	}

	it.size = it.stream->available();
	debug_d("Finished: URL: %s, Offset: 0x%X, Length: %u", it.url.c_str(), it.partition.address(), it.size);

	it.stream.release(); // the actual deletion will happen outside of this class
	currentItem++;

	if(currentItem < items.count()) {
		fetchNextItem();
	} else {
		downloadComplete();
	}

	return 0;
}

void HttpUpgrader::downloadComplete()
{
#if DEBUG_VERBOSE_LEVEL >= DBG
	debug_d("\r\nFirmware download finished!");
	for(unsigned i = 0; i < items.count(); i++) {
		auto& it = items[i];
		debug_d(" - item: %u, addr: 0x%X, size: 0x%X, url: %s", i, it.partition.address(), it.size, it.url.c_str());
	}
#endif

	if(updateDelegate) {
		updateDelegate(*this, true);
	}

	applyUpdate();
}

void HttpUpgrader::downloadFailed()
{
	debug_e("\r\nFirmware download failed..");
	if(updateDelegate) {
		updateDelegate(*this, false);
	}
	items.clear();
}

void HttpUpgrader::applyUpdate()
{
	items.clear();
	if(romSlot == NO_ROM_SWITCH) {
		debug_d("Firmware updated.");
		return;
	}

	// set to boot new rom and then reboot
	debug_d("Firmware updated, rebooting to rom %u...\r\n", romSlot);

	OtaManager.setBootPartition(items[romSlot].partition);
	System.restart();
}

} // namespace Network

} // namespace Ota
