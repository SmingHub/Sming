/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Ethernet.h
 *
 ****/

#pragma once

#include <WString.h>
#include <IpAddress.h>
#include <MacAddress.h>
#include <Delegate.h>

/**	@defgroup ethernet Ethernet Events Interface
 *	@brief	Event callback interface for Ethernet events
 *  @{
*/

/**
 * @brief Ethernet event code map
 * @note These codes are based on the ESP IDF public API.
 */
#define ETHERNET_EVENT_CODES_MAP(XX)                                                                                   \
	XX(Start, "Ethernet driver Started")                                                                               \
	XX(Stop, "Ethernet driver stopped")                                                                                \
	XX(Connected, "Ethernet link established")                                                                         \
	XX(Disconnected, "Ethernet link lost")

/**
 * @brief Ethernet event codes
 */
enum class EthernetEvent {
#define XX(tag, desc) tag,
	ETHERNET_EVENT_CODES_MAP(XX)
#undef XX
};

String toString(EthernetEvent event);

/**
 * @brief Ethernet class
 */
class Ethernet
{
public:
	/**
	 * @brief Delegate type for Ethernet events
	 * @param event Which event occurred
	 * @param mac Provided on 'Connected' event only
	 */
	using EventDelegate = Delegate<void(EthernetEvent event, MacAddress mac)>;

	/**
	 * @brief Delegate type for 'got IP address' event
	 */
	using GotIpDelegate = Delegate<void(IpAddress ip, IpAddress netmask, IpAddress gateway)>;

	bool begin();

	/**
	 * @brief Set callback for ethernet events
	 */
	void onEvent(EventDelegate callback)
	{
		eventCallback = callback;
	}

	/**
	 * @brief Set callback for 'station connected with IP address' event
	 */
	void onGotIp(GotIpDelegate callback)
	{
		gotIpCallback = callback;
	}

protected:
	EventDelegate eventCallback;
	GotIpDelegate gotIpCallback;
};

/** @} */
