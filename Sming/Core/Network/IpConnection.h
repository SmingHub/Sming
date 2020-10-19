/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * UdpConnection.h
 *
 ****/

/** @defgroup   ip IP
 *  @brief      Provides common IP functions
 *  @ingroup    networking
 *  @{
 */
#pragma once

#include <IpAddress.h>
#include <lwip/igmp.h>

class IpConnection
{
	/**
	 * @brief Uses IGPM to add a local IP to multicast group	 * @param localIp
	 * @param multicastIp
	 *
	 * @retval true on success
	 * @param localIp
	 * @param multicastIp
	 *
	 * @retval true on success
	 */
	bool joinMulticastGroup(IpAddress localIp, IpAddress multicastIp);

	/**
	 * @brief Uses IGPM to remove a local IP from multicast group
	 * @param localIp
	 * @param multicastIp
	 *
	 * @retval true on success
	 */
	bool leaveMulticastGroup(IpAddress localIp, IpAddress multicastIp);
};

/** @} */
