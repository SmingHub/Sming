/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * NtpClient.h
 *
 ****/

/** @defgroup   ntp Network Time Protocol client
 *  @brief      Provides NTP client
 *  @ingroup    datetime
 *  @ingroup    udp
 *  @{
 */
#pragma once

#include "UdpConnection.h"
#include "Platform/System.h"
#include "Timer.h"
#include "DateTime.h"

#define NTP_PORT 123
#define NTP_PACKET_SIZE 48
#define NTP_VERSION 4
#define NTP_MODE_CLIENT 3
#define NTP_MODE_SERVER 4

#define NTP_DEFAULT_SERVER F("pool.ntp.org")
#define NTP_DEFAULT_AUTOQUERY_SECONDS 30U // (10U * SECS_PER_MIN) ///< Refresh time if autoupdate set
#define NTP_MIN_AUTOQUERY_SECONDS 10U	 ///< Minimum autoquery interval
#define NTP_CONNECTION_TIMEOUT_MS 1666U   ///< Time to retry query when network connection unavailable
#define NTP_RESPONSE_TIMEOUT_MS 20000U	///< Time to wait before retrying NTP query

class NtpClient;

// Delegate constructor usage: (&YourClass::method, this)
typedef Delegate<void(NtpClient& client, time_t ntpTime)> NtpTimeResultDelegate;

/** @brief  NTP client class */
class NtpClient : protected UdpConnection
{
public:
	/** @brief  Instantiates NTP client object
     */
	NtpClient() : NtpClient(nullptr, NTP_DEFAULT_AUTOQUERY_SECONDS, nullptr)
	{
	}

	/** @brief  Instantiates NTP client object
     *  @param  onTimeReceivedCb Callback delegate to be called when NTP time result is received
     */
	NtpClient(NtpTimeResultDelegate onTimeReceivedCb)
		: NtpClient(nullptr, NTP_DEFAULT_AUTOQUERY_SECONDS, onTimeReceivedCb)
	{
	}

	/** @brief  Instantiates NTP client object
     *  @param  reqServer IP address or hostname of NTP server; nullptr to use default server
     *  @param  reqIntervalSeconds Quantity of seconds between NTP requests
     *  @param  onTimeReceivedCb Callback delegate to be called when NTP time result is received (Default: None)
     */
	NtpClient(const String& reqServer, unsigned reqIntervalSeconds, NtpTimeResultDelegate onTimeReceivedCb = nullptr);

	/** @brief  Request time from NTP server
     *  @note   Instigates request. Result is handled by NTP result handler function if defined
     */
	void requestTime();

	/** @brief  Set the NTP server
     *  @param  server IP address or hostname of NTP server
     */
	void setNtpServer(const String& server)
	{
		this->server = server;
	}

	/** @brief  Enable / disable periodic query
     *  @param  autoQuery True to enable periodic query of NTP server
     */
	void setAutoQuery(bool autoQuery);

	/** @brief  Set query period
     *  @param  seconds Period in seconds between periodic queries
     */
	void setAutoQueryInterval(unsigned seconds);

	/** @brief  Enable / disable update of system clock
     *  @param  autoUpdateClock True to update system clock with NTP result.
     */
	void setAutoUpdateSystemClock(bool autoUpdateClock)
	{
		autoUpdateSystemClock = autoUpdateClock;
	}

protected:
	/** @brief  Handle UDP message reception
     *  @param  buf Pointer to data buffer containing UDP payload
     *  @param  remoteIP IP address of remote host
     *  @param  remotePort Port number of remote host
     */
	void onReceive(pbuf* buf, IPAddress remoteIP, uint16_t remotePort) override;

	/** @brief  Send time request to NTP server
     *  @param  serverIp IP address of NTP server
     */
	void internalRequestTime(IPAddress serverIp);

	/** @brief Start the timer running
	 *  @param milliseconds Time to run in milliseconds
	 */
	void startTimer(uint32_t milliseconds)
	{
		debug_d("NtpClient::startTimer(%u)", milliseconds);
		timer.setIntervalMs(milliseconds);
		timer.startOnce();
	}

	void stopTimer()
	{
		debug_d("NtpClient::stopTimer()");
		timer.stop();
	}

protected:
	String server; ///< IP address or Hostname of NTP server

	NtpTimeResultDelegate delegateCompleted = nullptr; ///< NTP result handler delegate
	bool autoUpdateSystemClock = false;				   ///< True to update system clock with NTP time
	bool autoQueryEnabled = false;
	unsigned autoQuerySeconds = NTP_DEFAULT_AUTOQUERY_SECONDS;
	Timer timer; ///< Deals with timeouts, retries and autoquery updates
};

/** @} */
