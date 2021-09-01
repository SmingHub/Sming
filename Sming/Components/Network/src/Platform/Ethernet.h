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
#include <memory>

/**
 * @brief Ethernet event code map
 * @note These codes are based on the ESP IDF public API.
 */
#define ETHERNET_EVENT_MAP(XX)                                                                                         \
	XX(Start, "Ethernet driver Started")                                                                               \
	XX(Stop, "Ethernet driver stopped")                                                                                \
	XX(Connected, "Ethernet link established")                                                                         \
	XX(Disconnected, "Ethernet link lost")

namespace Ethernet
{
/**
 * @brief Ethernet event codes
 */
enum class Event {
#define XX(tag, desc) tag,
	ETHERNET_EVENT_MAP(XX)
#undef XX
};

/**
 * @brief Delegate type for Ethernet events
 * @param event Which event occurred
 * @param mac Provided on 'Connected' event only
 */
using EventDelegate = Delegate<void(Ethernet::Event event, MacAddress mac)>;

/**
 * @brief Delegate type for 'got IP address' event
 */
using GotIpDelegate = Delegate<void(IpAddress ip, IpAddress netmask, IpAddress gateway)>;

/**
 * @brief Configuration for Ethernet MAC
 */
struct MacConfig {
	int8_t smiMdcPin = 23;  //< SMI MDC GPIO number, -1 if not used
	int8_t smiMdioPin = 18; //< SMI MDIO GPIO number, -1 if not used
};

/**
 * @brief Link speed
 */
enum class Speed {
	MBPS10,
	MBPS100,
};

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

/**
 * @brief Abstract Service class
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
class Service
{
public:
	/**
	 * @brief Configure and start the ethernet service
	 * @param config MAC configuration
	 * @param phyFactory Factory class to manage PHY instance (if required)
	 *
	 * Applications should expect to receive Start and Connected events.
	 */
	virtual bool begin(const MacConfig& config, PhyFactory* phyFactory) = 0;

	/**
	 * @brief Tear down the ethernet connection
	 */
	virtual void end() = 0;

	/**
    * @brief Get MAC address
    */
	virtual MacAddress getMacAddress() const = 0;

	/**
	 * @brief Set MAC address
	 */
	virtual bool setMacAddress(const MacAddress& addr) = 0;

	/**
    * @brief Set speed of MAC
    */
	virtual bool setSpeed(Speed speed) = 0;

	/**
    * @brief Set duplex mode of MAC
    */
	virtual bool setFullDuplex(bool enable) = 0;

	/**
    * @brief Set link status of MAC
    */
	virtual bool setLinkState(bool up) = 0;

	/**
    * @brief Set MAC promiscuous mode
    */
	virtual bool setPromiscuous(bool enable) = 0;

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
	std::unique_ptr<Ethernet::PhyFactory> phyFactory;
	EventDelegate eventCallback;
	GotIpDelegate gotIpCallback;
};

} // namespace Ethernet

String toString(Ethernet::Event event);
String toLongString(Ethernet::Event event);
