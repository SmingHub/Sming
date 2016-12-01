/** @defgroup   ntp Network Time Protocol client
 *  @brief      Provides NTP client
 *  @ingroup    datetime
 *  @{
 */
#ifndef APP_NTPCLIENT_H_
#define APP_NTPCLIENT_H_

#include "UdpConnection.h"
#include "../Platform/System.h"
#include "../Timer.h"
#include "../SystemClock.h"
#include "../Platform/Station.h"
#include "../Delegate.h"

#define NTP_PORT 123
#define NTP_PACKET_SIZE 48
#define NTP_VERSION 4
#define NTP_MODE_CLIENT 3
#define NTP_MODE_SERVER 4

#define NTP_DEFAULT_SERVER "pool.ntp.org"
#define NTP_DEFAULT_QUERY_INTERVAL_SECONDS 600 // 10 minutes
#define NTP_RESPONSE_TIMEOUT_MS 20000 // 20 seconds

class NtpClient;

// Delegate constructor usage: (&YourClass::method, this)
typedef Delegate<void(NtpClient& client, time_t ntpTime)> NtpTimeResultDelegate;


/** @brief  NTP client class */
class NtpClient : protected UdpConnection
{
public:
    /** @brief  Instantiates NTP client object
     */
	NtpClient();

    /** @brief  Instantiates NTP client object
     *  @param  onTimeReceivedCb Callback delegate to be called when NTP time result is received
     */
	NtpClient(NtpTimeResultDelegate onTimeReceivedCb);

    /** @brief  Instantiates NTP client object
     *  @param  reqServer IP address or hostname of NTP server
     *  @param  reqIntervalSeconds Quantity of seconds between NTP requests
     *  @param  onTimeReceivedCb Callback delegate to be called when NTP time result is received (Default: None)
     */
	NtpClient(String reqServer, int reqIntervalSeconds, NtpTimeResultDelegate onTimeReceivedCb = nullptr);

	virtual ~NtpClient();

    /** @brief  Request time from NTP server
     *  @note   Instigates request. Result is handled by NTP result handler function if defined
     */
	void requestTime();

    /** @brief  Set the NTP server
     *  @param  server IP address or hostname of NTP server
     */
	void setNtpServer(String server);

    /** @brief  Enable / disable periodic query
     *  @param  autoQuery True to enable periodic query of NTP server
     */
	void setAutoQuery(bool autoQuery);

    /** @brief  Set query period
     *  @param  seconds Period in seconds between periodic queries
     */
	void setAutoQueryInterval(int seconds);

    /** @brief  Enable / disable update of system clock
     *  @param  autoUpdateClock True to update system clock with NTP result.
     */
	void setAutoUpdateSystemClock(bool autoUpdateClock);

protected:
    /** @brief  Handle UDP message reception
     *  @param  buf Pointer to data buffer containing UDP payload
     *  @param  remoteIP IP address of remote host
     *  @param  remotePort Port number of remote host
     */
	void onReceive(pbuf *buf, IPAddress remoteIP, uint16_t remotePort);

    /** @brief  Send time request to NTP server
     *  @param  serverIp IP address of NTP server
     */
	void internalRequestTime(IPAddress serverIp);

protected:
	String server = NTP_DEFAULT_SERVER; ///< IP address or Hostname of NTP server

	NtpTimeResultDelegate delegateCompleted = nullptr; ///< NTP result handler delegate
	bool autoUpdateSystemClock = false; ///< True to update system clock with NTP time

	Timer autoUpdateTimer; ///< Periodic query timer
	Timer timeoutTimer; ///< NTP message timeout timer
	Timer connectionTimer; ///< Wait for WiFi connection timer

    /** @brief  Handle DNS response
     *  @param  name Pointer to c-string containing hostname
     *  @param  ip Ponter to IP address
     *  @param  arg Pointer to the NTP client object that made the DNS request
     *  @note   This function is called when a DNS query is serviced
     */
	static void staticDnsResponse(const char *name, struct ip_addr *ip, void *arg);
};

/** @} */
#endif /* APP_NTPCLIENT_H_ */
