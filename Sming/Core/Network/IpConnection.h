/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * IpConnection.h
 *
 ****/

#pragma once

#include <IpAddress.h>

/** @defgroup   ip IP
 *  @brief      Provides common IP functions
 *  @ingroup    networking
 *  @{
 */

class IpConnection
{
public:
	/**
	 * @brief Uses IGMP to add a local network interface to multicast group
	 * @param localIp Address identifying network interface
	 * @param multicastIp The multicast group address
	 *
	 * @retval true on success
	 */
	bool joinMulticastGroup(IpAddress localIp, IpAddress multicastIp);

	/**
	 * @brief Uses IGMP to add all local network interfaces to multicast group
	 * @param multicastIp The multicast group address
	 *
	 * @retval true on success
	 */
	bool joinMulticastGroup(IpAddress multicastIp)
	{
		return joinMulticastGroup(INADDR_NONE, multicastIp);
	}

	/**
	 * @brief Uses IGMP to remove a local network interface from multicast group
	 * @param localIp Address identifying network interface
	 * @param multicastIp The multicast group address
	 *
	 * @retval true on success
	 */
	bool leaveMulticastGroup(IpAddress localIp, IpAddress multicastIp);

	/**
	 * @brief Uses IGMP to remove all local network interfaces from multicast group
	 * @param multicastIp The multicast group address
	 *
	 * @retval true on success
	 */
	bool leaveMulticastGroup(IpAddress multicastIp)
	{
		return leaveMulticastGroup(INADDR_NONE, multicastIp);
	}
};

/** @} */
