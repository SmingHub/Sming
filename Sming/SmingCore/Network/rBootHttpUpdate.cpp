/*
 * rBootHttpUpdate.cpp
 *
 *  Created on: 2015/09/03.
 *      Author: Richard A Burton & Anakod
 */

#include "rBootHttpUpdate.h"
#include "../Platform/System.h"
#include "URL.h"
#include "../Platform/WDT.h"

rBootHttpUpdate::rBootHttpUpdate() {
	currentItem = 0;
	romSlot = NO_ROM_SWITCH;
	updateDelegate = nullptr;
}

rBootHttpUpdate::~rBootHttpUpdate() {
}

void rBootHttpUpdate::addItem(int offset, String firmwareFileUrl) {
	rBootHttpUpdateItem add;
	add.targetOffset = offset;
	add.url = firmwareFileUrl;
	add.size = 0;
	items.add(add);
}

void rBootHttpUpdate::start() {
	timer.initializeMs(500, TimerDelegate(&rBootHttpUpdate::onTimer, this)).start();
}

void rBootHttpUpdate::switchToRom(uint8 romSlot) {
	this->romSlot = romSlot;
}

void rBootHttpUpdate::setCallback(otaUpdateDelegate reqUpdateDelegate) {
	setDelegate(reqUpdateDelegate);
}

void rBootHttpUpdate::setDelegate(otaUpdateDelegate reqUpdateDelegate) {
	this->updateDelegate = reqUpdateDelegate;
}

void rBootHttpUpdate::updateFailed() {
	timer.stop();
	items.clear();
	debugf("\r\nFirmware download failed..");
	if (updateDelegate) updateDelegate(false);
}

void rBootHttpUpdate::onTimer() {
	
	if (TcpClient::isProcessing()) return; // Will wait
	
	if (TcpClient::getConnectionState() == eTCS_Successful) {
		
		if (!isSuccessful()) {
			updateFailed();
			return;
		}
		
		currentItem++;
		if (currentItem >= items.count()) {
			debugf("\r\nFirmware download finished!");
			for (int i = 0; i < items.count(); i++) {
				debugf(" - item: %d, addr: %X, len: %d bytes", i, items[i].targetOffset, items[i].size);
			}
			
			applyUpdate();
			return;
		}
		
	} else if (TcpClient::getConnectionState() == eTCS_Failed) {
		updateFailed();
		return;
	}
	
	rBootHttpUpdateItem &it = items[currentItem];
	debugf("Download file:\r\n    (%d) %s -> %X", currentItem, it.url.c_str(), it.targetOffset);
	rBootWriteStatus = rboot_write_init(items[currentItem].targetOffset);
	startDownload(URL(it.url), eHCM_UserDefined, NULL);
}

void rBootHttpUpdate::writeRawData(pbuf* buf, int startPos) {
	pbuf *cur = buf;
	while (cur != NULL && cur->len > 0 && !writeError) {
		uint8* ptr = (uint8*) cur->payload + startPos;
		int len = cur->len - startPos;
		writeError = !rboot_write_flash(&rBootWriteStatus, ptr, len);
		if (writeError) {
			debugf("Write Error!");
		}
		items[currentItem].size += len;
		cur = cur->next;
		startPos = 0;
	}
}

void rBootHttpUpdate::applyUpdate() {
	timer.stop();
	items.clear();
	if (romSlot == NO_ROM_SWITCH) {
		debugf("Firmware updated.");
		if (updateDelegate) updateDelegate(true);
	} else {
		// set to boot new rom and then reboot
		debugf("Firmware updated, rebooting to rom %d...\r\n", romSlot);
		rboot_set_current_rom(romSlot);
		System.restart();
	}
	return;
}
