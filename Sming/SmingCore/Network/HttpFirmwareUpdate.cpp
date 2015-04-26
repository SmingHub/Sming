/*
 * HttpFirmwareUpdate.cpp
 *
 *  Created on: 26 апр. 2015 г.
 *      Author: Anakod
 */

#include "HttpFirmwareUpdate.h"
#include "../../Services/SpifFS/spiffs.h"
#include "../Interrupts.h"
#include "../Platform/System.h"
#include "../Platform/WDT.h"
#include "URL.h"

HttpFirmwareUpdate::HttpFirmwareUpdate()
{
	memset(&timer, 0, sizeof(ETSTimer));
	currentItem = 0;
}

HttpFirmwareUpdate::~HttpFirmwareUpdate()
{
	ets_timer_disarm(&timer);
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
	spiffs_unmount();
	spiffs_format_internal();
	spiffs_config cfg = spiffs_get_storage_config();
	pos = cfg.phys_addr;

	ets_timer_setfn(&timer, (os_timer_func_t *)staticOnTimer, this);
	ets_timer_arm_new(&timer, 500, true, 1); // msec
}

void HttpFirmwareUpdate::staticOnTimer(void* ptrSelf)
{
	HttpFirmwareUpdate* self = (HttpFirmwareUpdate*)ptrSelf;
	self->onTimer();
}

void HttpFirmwareUpdate::onTimer()
{
	if (HttpClient::isProcessing()) return; // Will wait

	if (HttpClient::getState() == eTCS_Successful)
	{
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

void IRAM_ATTR HttpFirmwareUpdate::applyUpdate()
{
	const int unit = INTERNAL_FLASH_SECTOR_SIZE;
	char buf[unit];

	ets_wdt_disable();
	noInterrupts();

	ets_uart_printf("Firmware upgrade started\n");

	for (int i = 0; i < items.size(); i++)
	{
		uint32_t to = items[i].targetOffset;
		uint32_t from = items[i].flash - INTERNAL_FLASH_START_ADDRESS;
		int size = items[i].size;

		ets_uart_printf("Start item %d: 0x%X -> 0x%X %d, align: %d\n", i, from, to, size, ((uint32_t)buf % 4) == 0);
		while (size > 0)
		{
			int sect = to / unit;
			int cur = unit;
			if (cur > size) cur = size;
			ets_uart_printf("write: 0x%X -> 0x%X (sect: %d), %d (%d)\n", from, to, sect, cur, size);
			spi_flash_erase_sector(sect);
			//ets_uart_printf("ers.");
			spi_flash_read(from, (uint32*)buf, cur);
			//ets_uart_printf("read.");
			spi_flash_write(to, (uint32*)buf, cur);
			//ets_uart_printf("wr.\r\n");
			from += cur;
			to += cur;
			size -= cur;
		}
	}

	ets_uart_printf("Firmware upgrade finished\n");

	((void (*)(void))0x40000080)(); // Hardcore reset vector

	ets_wdt_enable();
	while (1)
		; // Reboot anyway!
}
