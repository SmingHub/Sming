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



class NtpClient : protected UdpConnection
{
public:
	NtpClient();
	NtpClient(NtpTimeResultDelegate onTimeReceivedCb);
	NtpClient(String reqServer, int reqIntervalSeconds, NtpTimeResultDelegate onTimeReceivedCb = nullptr);
	virtual ~NtpClient();

	void requestTime();
	
	void setNtpServer(String server);
	
	void setAutoQuery(bool autoQuery);
	void setAutoQueryInterval(int seconds);

	void setAutoUpdateSystemClock(bool autoUpdateClock);

protected:
	void onReceive(pbuf *buf, IPAddress remoteIP, uint16_t remotePort);
	void internalRequestTime(IPAddress serverIp);

protected: 
	String server = NTP_DEFAULT_SERVER;

	NtpTimeResultDelegate delegateCompleted = nullptr;
	bool autoUpdateSystemClock = false;
		
	Timer autoUpdateTimer;
	Timer timeoutTimer;
	Timer connectionTimer;
		
	static void staticDnsResponse(const char *name, struct ip_addr *ip, void *arg);		
};

#endif /* APP_NTPCLIENT_H_ */
