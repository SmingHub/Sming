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
#define ETHERNET_EVENT_MAP(XX)                                                                                         \
	XX(Start, "Ethernet driver Started")                                                                               \
	XX(Stop, "Ethernet driver stopped")                                                                                \
	XX(Connected, "Ethernet link established")                                                                         \
	XX(Disconnected, "Ethernet link lost")

/**
 * @brief Ethernet event codes
 */
enum class EthernetEvent {
#define XX(tag, desc) tag,
	ETHERNET_EVENT_MAP(XX)
#undef XX
};

String toString(EthernetEvent event);
String toLongString(EthernetEvent event);

/**
 * @brief Ethernet class
 *
 * Provides a common implementation for TCP/IP ethernet support.
 * 
 * An Ethernet interface requires a MAC layer plus PHY.
 *
 * The ESP32, for example, contains a MAC but requires an external PHY which must
 * be configured separately. The PHY is therefore a separate class which is used
 * to configure the MAC.
 *
 * Other solutions, such as the W5500, contain MAC+PHY so handle this internally.
 *
 * Ethernet implementations should provide appropriate setup methods which are called by the application
 * before invoking `begin()`.
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

	/**
	 * @brief Constructed PHY instance
	 */
	struct PhyInstance;

	/**
	 * @brief PHY configuration
	 */
	struct PhyConfig {
		int8_t phyAddr = -1;			///< PHY address, set -1 to enable PHY address detection at initialization stage
		int8_t resetPin = 5;			///< Reset GPIO number, -1 means no hardware reset */
		uint16_t resetTimeout = 100;	///< Reset timeout value in milliseconds
		uint16_t autoNegTimeout = 4000; ///< Auto-negotiation timeout in milliseconds
	};

	/**
	 * @brief Virtual class used to construct a specific PHY instance
	 */
	class PhyFactory
	{
	public:
		using PhyInstance = Ethernet::PhyInstance;

		PhyFactory(const PhyConfig& config) : config(config)
		{
		}

		virtual PhyInstance* create() = 0;

		virtual void destroy(PhyInstance* inst) = 0;

	protected:
		PhyConfig config;
	};

	class NullPhy : public PhyFactory
	{
	public:
		PhyInstance* create() override
		{
			return nullptr;
		};

		void destroy(PhyInstance* inst) override
		{
			assert(inst == nullptr);
		}
	};

	/**
	 * @brief Configure and start the ethernet service
	 * 
	 * Applications should expect to receive Start and Connected events.
	 */
	virtual bool begin(PhyFactory* phyFactory) = 0;

	/**
	 * @brief Tear down the ethernet connection
	 */
	virtual void end() = 0;

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
