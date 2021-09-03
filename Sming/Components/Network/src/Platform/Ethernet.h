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
using EventDelegate = Delegate<void(Ethernet::Event event)>;

/**
 * @brief Delegate type for 'got IP address' event
 */
using GotIpDelegate = Delegate<void(IpAddress ip, IpAddress netmask, IpAddress gateway)>;

/**
 * @brief Use default pin for platform
 */
constexpr int8_t PIN_DEFAULT{-2};

/**
 * @brief Do not configure this pin
 *
 * Only applies if pin is optional, otherwise it will be interpreted as 'auto detect'.
 */
constexpr int8_t PIN_UNUSED{-1};

/**
 * @brief Configuration for Ethernet MAC
 */
struct MacConfig {
	int8_t smiMdcPin = PIN_DEFAULT;  //< SMI MDC GPIO number
	int8_t smiMdioPin = PIN_DEFAULT; //< SMI MDIO GPIO number
};

/**
 * @brief Link speed
 */
enum class Speed {
	MBPS10,
	MBPS100,
};

/**
 * @brief Constructed PHY instance. An opaque, implementation-specific type.
 */
struct PhyInstance;

/**
 * @brief Automatically detect PHY address during initialization
 */
constexpr int8_t PHY_ADDR_AUTO{-1};

/**
 * @brief PHY configuration
 */
struct PhyConfig {
	int8_t phyAddr = PHY_ADDR_AUTO; ///< PHY address
	int8_t resetPin = PIN_UNUSED;   ///< Reset GPIO number */
	uint16_t resetTimeout = 100;	///< Reset timeout value in milliseconds
	uint16_t autoNegTimeout = 4000; ///< Auto-negotiation timeout in milliseconds
};

/**
 * @brief Virtual class used to construct a specific PHY instance
 *
 * Applications provide an instance of this factory class so that the Service
 * can create and configure it at the correct point in initialisation or teardown.
 */
class PhyFactory
{
public:
	using PhyInstance = Ethernet::PhyInstance;

	/**
	 * @brief Called by the Service to construct a PHY instance.
	 */
	virtual PhyInstance* create(const PhyConfig& config) = 0;

	/**
	 * @brief Called by the Service to destroy a PHY instance.
	 */
	virtual void destroy(PhyInstance* inst) = 0;
};

/**
 * @brief Service configuration options
 */
struct Config {
	MacConfig mac;
	PhyConfig phy;
};

/**
 * @brief Abstract Service class
 *
 * Provides a common implementation for TCP/IP ethernet support.
 * 
 * An Ethernet interface requires a MAC layer plus PHY.
 *
 * The ESP32, for example, contains a MAC but requires an external PHY.
 * Other solutions, such as the W5500, contain MAC+PHY and require the correct
 * PhyFactory to work.
 *
 * Ethernet implementations should provide appropriate setup methods which are called by the application
 * before invoking `begin()`.
 */
class Service
{
public:
	/**
	 * @brief Configure and start the ethernet service
	 * @param config Configuration options
	 *
	 * Applications should expect to receive Start and Connected events following this call.
	 */
	virtual bool begin(const Config& config) = 0;

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
	 * @brief Set DHCP hostname
	 */
	virtual void setHostname(const String& hostname) = 0;

	/**
	 * @brief Get DHCP hostname
	 */
	virtual String getHostname() const = 0;

	/**
	 * @brief Get current IP address
	 */
	virtual IpAddress getIP() const = 0;

	/**
	 * @brief Set static IP address
	 */
	virtual bool setIP(IpAddress address, IpAddress netmask, IpAddress gateway) = 0;

	/**
	 * @brief Determine if DHCP is active for this interface
	 */
	virtual bool isEnabledDHCP() const = 0;

	/**
	 * @brief Enable/disable DHCP on this interface
	 */
	virtual bool enableDHCP(bool enable) = 0;

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

} // namespace Ethernet

String toString(Ethernet::Event event);
String toLongString(Ethernet::Event event);
