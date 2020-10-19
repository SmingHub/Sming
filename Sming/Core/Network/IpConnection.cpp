#include "IpConnection.h"

bool IpConnection::joinMulticastGroup(IpAddress localIp, IpAddress multicastIp)
{
	return (igmp_joingroup(localIp, multicastIp) == ERR_OK);
}

bool IpConnection::leaveMulticastGroup(IpAddress localIp, IpAddress multicastIp)
{
	return (igmp_leavegroup(localIp, multicastIp) == ERR_OK);
}
