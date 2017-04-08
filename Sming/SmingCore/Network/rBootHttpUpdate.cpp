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

void rBootHttpUpdate::setCallback(OtaUpdateDelegate reqUpdateDelegate) {
	setDelegate(reqUpdateDelegate);
}

void rBootHttpUpdate::setDelegate(OtaUpdateDelegate reqUpdateDelegate) {
	this->updateDelegate = reqUpdateDelegate;
}

void rBootHttpUpdate::updateFailed() {
	timer.stop();
	debugf("\r\nFirmware download failed..");
	if (updateDelegate) updateDelegate(*this, false);
	items.clear();
}

void rBootHttpUpdate::onTimer() {
	
	if (TcpClient::isProcessing()) return; // Will wait
	
	if (TcpClient::getConnectionState() == eTCS_Successful) {

		//  always call writeEnd()
		if (!writeEnd()) {
			debugf("final checks failed!");
			writeError = 1;
		}    
		
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
	writeInit();
	startDownload(URL(it.url), eHCM_UserDefined, NULL);
}

err_t rBootHttpUpdate::onResponseBody(const char *at, size_t length) {
	debugf("onResponseBody: %d", length);
	writeError = !writeFlash((uint8 *)at, length);
	if (writeError) {
		return -1;
	}
	items[currentItem].size += length;

	return ERR_OK;
}

void rBootHttpUpdate::writeInit() {
	rBootWriteStatus = rboot_write_init( items[currentItem].targetOffset );
}

bool rBootHttpUpdate::writeFlash(const u8 *data, u16 size) {
	return rboot_write_flash(&rBootWriteStatus, (u8 *) data, size );
}

bool rBootHttpUpdate::writeEnd() {
	return rboot_write_end(&rBootWriteStatus);
}

void rBootHttpUpdate::applyUpdate() {
	timer.stop();
	if (romSlot == NO_ROM_SWITCH) {
		debugf("Firmware updated.");
		if (updateDelegate) updateDelegate(*this, true);
		items.clear();
	} else {
		// set to boot new rom and then reboot
		debugf("Firmware updated, rebooting to rom %d...\r\n", romSlot);
		rboot_set_current_rom(romSlot);
		System.restart();
	}
	return;
}

rBootHttpUpdateItem rBootHttpUpdate::getItem(unsigned int index) {
	return items.elementAt(index);
}
