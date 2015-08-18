/*
 * HttpFirmwareUpdate.cpp
 *
 *  Created on: 26 апр. 2015 г.
 *      Author: Anakod
 */

#include "HttpFirmwareUpdate.h"
#include "../../Services/SpifFS/spiffs.h"
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
	spiffs_format_internal();
	spiffs_config cfg = spiffs_get_storage_config();
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
				debugf("\t item: 0x%X 0x%X %d bytes", items[i].targetOffset, items[i].flash - INTERNAL_FLASH_START_ADDRESS, items[i].size);

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
//	uint32_t sect = flashmem_get_sector_of_address(pos);
//	sect++;
//	pos = INTERNAL_FLASH_START_ADDRESS + sect * INTERNAL_FLASH_SECTOR_SIZE;
	it.flash = pos;
	debugf("Download file:\r\n    (%d) %s -> 0x%X", currentItem, it.url.c_str(), it.targetOffset);
	startDownload(URL(it.url), eHCM_UserDefined, NULL);
}

void HttpFirmwareUpdate::writeRawData(pbuf* buf, int startPos)
{
	pbuf *cur = buf;
	while (cur != NULL && cur->len > 0 && !writeError)
	{
		char* ptr = (char*) cur->payload + startPos;
		int len = cur->len - startPos;
		int res = writeFlash(ptr, pos, len);
		//debugf("Write 0x%X %d %d", pos, len, res);
		pos += res;
		writeError |= (res != len);
		if (writeError)
			debugf("WriteError %d != %d", res, len);
		cur = cur->next;
		startPos = 0;
	}
}

uint32_t HttpFirmwareUpdate::writeFlash(char* data, uint32_t pos, int size)
{
	return flashmem_write(data, pos, size);
}

void HttpFirmwareUpdate::applyUpdate()
{
	// From begin of first to the end of last sector
	int size = items[items.count() - 1].targetOffset + items[items.count() - 1].size - items[0].targetOffset;
	System.applyFirmwareUpdate(items[0].flash - INTERNAL_FLASH_START_ADDRESS, items[0].targetOffset, size);
	return;
}
