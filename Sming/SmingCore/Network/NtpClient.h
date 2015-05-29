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

#define NTP_SERVER_DEFAULT "pool.ntp.org"

#define NTP_DEFAULT_AUTO_UPDATE_INTERVAL 600000 // 10 minutes

class NtpClient;

// Define Callback and Delegate
DELEGATE_CALLBACK (NtpTimeResult,void,NtpClient& client, time_t ntpTime)

class NtpClient : protected UdpConnection
{
public:
	
	NtpClient();
	NtpClient(NtpTimeResultCallback onTimeReceivedCb);
	NtpClient(String reqServer, int reqIntervalSeconds, NtpTimeResultCallback onTimeReceivedCb);
	NtpClient(String reqServer, int reqIntervalSeconds, NtpTimeResultDelegate delegateFunction = NULL);
	virtual ~NtpClient();

	void requestTime();
	
	void setNtpServer(String server);
	void setNtpServer(IPAddress adress);
	
	void setAutoQuery(bool autoQuery);
	void setAutoQueryInterval(int seconds);

	void setAutoUpdateSystemClock(bool autoUpdateClock);

protected:
	int resolveServer();
	void onReceive(pbuf *buf, IPAddress remoteIP, uint16_t remotePort);
	
protected: 
	String server = NTP_SERVER_DEFAULT;
	IPAddress serverAddress = (uint32_t)0;
	NtpTimeResultDelegate delegateCompleted = nullptr;
	bool autoUpdateSystemClock = false;
		
	Timer autoUpdateTimer;
	Timer connectionTimer;
		
	static void staticDnsResponse(const char *name, struct ip_addr *ip, void *arg);		
};

#endif /* APP_NTPCLIENT_H_ */
