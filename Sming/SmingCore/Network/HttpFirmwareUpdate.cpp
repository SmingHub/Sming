/*
 * HttpFirmwareUpdate.cpp
 *
 *  Created on: 26 ���. 2015 �.
 *      Author: Anakod
 */

#include "HttpFirmwareUpdate.h"
#include "../../Services/SpifFS/spiffs_sming.h"
#include "../Platform/System.h"
#include "URL.h"
#include "../Platform/WDT.h"

HttpFirmwareUpdate::HttpFirmwareUpdate()
{
	currentItem = 0;
	pos = 0;
}

HttpFirmwareUpdate::~HttpFirmwareUpdate()
{
}

void HttpFirmwareUpdate::addItem(int offset, String firmwareFileUrl)
{
	HttpFirmwareUpdateItem add;
	add.targetOffset = offset;
	add.url = firmwareFileUrl;
	items.add(add);
}

void HttpFirmwareUpdate::start()
{
	WDT.enable(false);
	spiffs_unmount();
	spiffs_config cfg = spiffs_get_storage_config();
	spiffs_format_internal(&cfg);
	pos = cfg.phys_addr;

	timer.initializeMs(500, TimerDelegate(&HttpFirmwareUpdate::onTimer, this)).start();
}

void HttpFirmwareUpdate::updateFailed()
{
	debugf("\r\nFirmware download failed..");
	timer.stop();
}

void HttpFirmwareUpdate::onTimer()
{
	if (HttpClient::isProcessing()) return; // Will wait

	if (TcpClient::getConnectionState() == eTCS_Successful)
	{
		if (!isSuccessful())
		{
			updateFailed();
			return;
		}

		items[currentItem].size = pos - items[currentItem].flash;
		currentItem++;
		if (currentItem >= items.count())
		{
			debugf("\r\nFirmware download finished!");
			for (int i = 0; i < items.count(); i++)
				debugf("\t item: 0x%X 0x%X %d bytes", items[i].targetOffset, items[i].flash, items[i].size);

			applyUpdate();
			return;
		}
		HttpFirmwareUpdateItem &prev = items[currentItem - 1];
		HttpFirmwareUpdateItem &cur = items[currentItem];
		int jmp = cur.targetOffset - prev.targetOffset - prev.size;
		pos += jmp;
		debugf("jump: %d", jmp);
	}
	else if (TcpClient::getConnectionState() == eTCS_Failed)
	{
		updateFailed();
		return;
	}

	HttpFirmwareUpdateItem &it = items[currentItem];
	it.flash = pos;
	debugf("Download file:\r\n    (%d) %s -> 0x%X", currentItem, it.url.c_str(), it.targetOffset);
	startDownload(URL(it.url), eHCM_UserDefined, NULL);
}

err_t HttpFirmwareUpdate::onResponseBody(const char *at, size_t length)
{
	int res = writeFlash(at, pos, length);
	writeError |= (res != length);
	if (writeError) {
		debugf("WriteError %d != %d", res, length);
		return -1;
	}

	pos += res;

	return 0;
}

uint32_t HttpFirmwareUpdate::writeFlash(const char* data, uint32_t pos, int size)
{
	return flashmem_write(data, pos, size);
}

void HttpFirmwareUpdate::applyUpdate()
{
	// From begin of first to the end of last sector
	int size = items[items.count() - 1].targetOffset + items[items.count() - 1].size - items[0].targetOffset;
	System.applyFirmwareUpdate(items[0].flash, items[0].targetOffset, size);
	return;
}
