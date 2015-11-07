/*
 * RBoot.cpp
 *
 *  Created on: 5 nov. 2015
 *      Author: Herman
 */

#include "RBootClass.h"

RBoot::RBoot()
{
}

RBoot::~RBoot()
{
	// TODO Auto-generated destructor stub
}

int RBoot::getCurrentRom()
{
	return rboot_get_current_rom();
}

bool RBoot::startRom(int reqSlot)
{
	rboot_config bootconf;
	bootconf = rboot_get_config();
	if (reqSlot >= bootconf.count)
	{
		return false;
	}
	rboot_set_current_rom(reqSlot);
	System.restart();
	return true; // Will never be executed;
}

bool RBoot::loadRom(uint8 loadType, uint8 reqSlot, String reqURL)
{
	// romStatus = loading
	rboot_config bootconf;
	bootconf = rboot_get_config();

	if ((reqSlot >= bootconf.count) || (getCurrentRom() == reqSlot))
	{
		return false;
	};
	httpUpdate = new rBootHttpUpdate();
	httpUpdate->setCallback(otaUpdateDelegate(&RBoot::OtaUpdateDelegate,this));
	httpUpdate->addItem(bootconf.roms[reqSlot], reqURL);
	httpUpdate->start();

	return true;
}

void RBoot::setDelegate (RBootDelegate reqDelegate)
{
	rbootDelegate = reqDelegate;
}

bool RBoot::mountSpiffs(uint8 reqSlot)
{
	rboot_config bootconf;
	bootconf = rboot_get_config();

	if ((reqSlot >= bootconf.count) || (getCurrentRom() == reqSlot))
	{
		return false;
	};

	debugf("trying to mount spiffs at %x, length %d", bootconf.roms[reqSlot] + 0x40200000, 65536);
	spiffs_mount_manual(bootconf.roms[reqSlot] + 0x40200000, 65536);
	return true;
}

void RBoot::unmoutSpiffs()
{
	spiffs_unmount();
}

void RBoot::OtaUpdateDelegate(bool result)
{
	int romStatus;
	romStatus = result ? 0 : 1; // valid : error
	debugf("Delegate Called result = %d", result);

	if (rbootDelegate)
	{
		rbootDelegate(result ? 0 : 1);
	}
}

void RBoot::processRBootCommands(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ' , commandToken);

	if (numToken == 1)
	{
		commandOutput->printf("RBoot Commands available : \r\n");
		commandOutput->printf("info : Show Rominfo\r\n");
		commandOutput->printf("load : Load ROM contents \r\n");
		commandOutput->printf("start : Restart to <ROM>\r\n");
		commandOutput->printf("mount : Mount Spiffs from <ROM>\r\n");
		commandOutput->printf("unmount : Unmount spiffs filesystem\r\n");
	}
	else
	{
		if (commandToken[1] == "load")
		{
			if (numToken != 4)
			{
				commandOutput->printf("Usage : rboot load <Rom#> <URL>\r\n");
			}
			else
			{
				int rom = commandToken[2].toInt();
				debugf("Loading Rom %s",commandToken[3].c_str());
				if (loadRom(1,rom,commandToken[3]))
				{
					commandOutput->printf("Loading %s to ROM%d started\r\n",commandToken[3].c_str(),rom);
				}
				else
				{
					commandOutput->printf("Loading %s to ROM%d failed\r\n",commandToken[3].c_str(),rom);
				}
			}
		}
		else if (commandToken[1] == "start")
		{
			int rom = getCurrentRom();
			if (numToken == 3)
			{
				rom = commandToken[2].toInt();
			}
			if (!startRom(rom))
			{
				commandOutput->printf("Invalid Rom for starting\r\n");
			}
		}
		else if (commandToken[1] == "mount")
		{
			if (SPIFFS_mounted(&_filesystemStorageHandle))
			{
				commandOutput->printf("Spiffs filesystem already mounted\r\n");
			}
			else
			if (numToken != 3)
			{
				commandOutput->printf("Usage : rboot spiffs <slot#> \r\n");
			}
			else
			{
				int slot = commandToken[2].toInt();
				if (mountSpiffs(slot))
				{
					commandOutput->printf("Spiffs on ROM%d mounted\r\n",slot);
				}
				else
				{
					commandOutput->printf("Mounting spiffs on ROM%d failed\r\n",slot);
				}
			}

		}
		else if (commandToken[1] == "unmount")
		{
			unmoutSpiffs();
			commandOutput->printf("Spiffs filesystem unmounted\r\n");
		}
		else if (commandToken[1] == "info")
		{
			showRomInfo(commandOutput);
		}
		else if (commandToken[1] == "ls")
		{
			showSpiffs(commandOutput);
		}
		else
		{
			commandOutput->printf("Unknown rboot command\r\n");
		}

	}
}

void RBoot::initCommand()
{
	commandHandler.registerCommand(CommandDelegate("rboot","Rboot related commands","RBoot",commandFunctionDelegate(&RBoot::processRBootCommands,this)));
}

void RBoot::showSpiffs(CommandOutput* commandOutput)
{
	if (!SPIFFS_mounted(&_filesystemStorageHandle))
	{
		commandOutput->printf("No Spiffs filesystem mounted\r\n");
	}
	else
	{
		Vector<String> files = fileList();
		commandOutput->printf("filecount %d\r\n", files.count());
		for (unsigned int i = 0; i < files.count(); i++)
		{
			commandOutput->println(files[i]);
		}
	}

}


void RBoot::showRomInfo(CommandOutput* commandOutput)
{

	rboot_config bootconf;
	bootconf = rboot_get_config();
	commandOutput->printf("\r\nROM Information\r\n");
	commandOutput->printf("Current ROM = %d\r\n",getCurrentRom());
	commandOutput->printf("Rboot Version %d\r\n",bootconf.version);
	for (int i = 0; i < bootconf.count;i++)
	{
		commandOutput->printf("ROM%d :  value = %d, %X\r\n", i, bootconf.roms[i],bootconf.roms[i]);
	}
}

