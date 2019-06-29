/*
 * CommandDelegate.cpp
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#include "CommandDelegate.h"

CommandDelegate::CommandDelegate()
{

}

CommandDelegate::CommandDelegate(String reqName, String reqHelp, String reqGroup, CommandFunctionDelegate reqFunction)
: commandName(reqName), commandHelp(reqHelp), commandGroup(reqGroup), commandFunction(reqFunction)
{
}

CommandDelegate::~CommandDelegate()
{
	// TODO Auto-generated destructor stub
}

