/*
 * RBoot.h
 *
 *  Created on: 5 nov. 2015
 *      Author: Herman
 */

#ifndef SMINGCORE_RBOOT_H_
#define SMINGCORE_RBOOT_H_


#include "../SmingCore/Network/rBootHttpUpdate.h"

#include "../Wiring/WiringFrameworkDependencies.h"
#include "../Services/CommandProcessing/CommandProcessingIncludes.h"
#include <rboot-api.h>
#include "FileSystem.h"

typedef struct
{
	uint32 StartAddress;     // Keep always as first to prevent updates to core rboot
	uint8 RomType;           // -> AppCode, SpiffyFS
	uint32 Size;
	uint8 Status;
} RBootRomInfo;

typedef Delegate<void(int resultCode)> RBootDelegate;

class RBoot
{
public:
	RBoot();
	virtual ~RBoot();
	bool loadRom(uint8 loadType, uint8 reqSlot, String reqURL);

	void OtaUpdateDelegate(bool result);
	int getCurrentRom();
	bool startRom(int reqSlot);

	void initCommand();
	void processRBootCommands(String commandLine, CommandOutput* commandOutput);
	void showRomInfo(CommandOutput* commandOutput);
	void showSpiffs(CommandOutput* commandOutput);

	bool mountSpiffs(uint8 reqRom);
	void unmoutSpiffs();

	void setDelegate (RBootDelegate reqDelegate);

	otaUpdateDelegate updateDelegate;
	rBootHttpUpdate* httpUpdate = 0;

	RBootDelegate rbootDelegate;
};

#endif /* SMINGCORE_RBOOT_H_ */
