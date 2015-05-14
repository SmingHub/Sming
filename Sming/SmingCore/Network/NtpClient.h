#ifndef APP_NTPCLIENT_H_
#define APP_NTPCLIENT_H_

#include "UdpConnection.h"
#include "../Platform/System.h"

#define NTP_PORT 123
#define NTP_PACKET_SIZE 48
#define NTP_VERSION 4
#define NTP_MODE_CLIENT 3
#define NTP_MODE_SERVER 4

#define NTP_LISTEN_PORT 57001
#define DEFAULT_AUTO_UPDATE_INTERVAL 600000 // 10 minutes


class NtpClient;
typedef void (*NtpTimeResultCallback)(NtpClient& client, uint32_t unixTime);


class NtpClient : protected UdpConnection
{
public:
	
	NtpClient(NtpTimeResultCallback onTimeReceivedCb);
	virtual ~NtpClient();

	void requestTime();
	void setAutoQuery(bool autoQuery);
	void setAutoQueryInterval(int seconds);
		
protected:
	int init();
	void onReceive(pbuf *buf, IPAddress remoteIP, uint16_t remotePort);
	
protected: 
	IPAddress serverAddress;
	NtpTimeResultCallback onCompleted;
		
	os_timer_t autoUpdateTimer;
	bool autoUpdate;
	unsigned int autoUpdateInteval;
		
	static void staticDnsResponse(const char *name, struct ip_addr *ip, void *arg);		
	static void staticAutoUpdateCallback(void *arg);
	
};

#endif /* APP_NTPCLIENT_H_ */
