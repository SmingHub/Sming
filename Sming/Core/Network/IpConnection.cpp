/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * IpConnection.cpp
 *
 ****/

#include "IpConnection.h"
#include <lwip/igmp.h>

bool IpConnection::joinMulticastGroup(IpAddress localIp, IpAddress multicastIp)
{
	return (igmp_joingroup(localIp, multicastIp) == ERR_OK);
}

bool IpConnection::leaveMulticastGroup(IpAddress localIp, IpAddress multicastIp)
{
	return (igmp_leavegroup(localIp, multicastIp) == ERR_OK);
}
