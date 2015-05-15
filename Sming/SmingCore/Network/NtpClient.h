#ifndef APP_NTPCLIENT_H_
#define APP_NTPCLIENT_H_

#include "UdpConnection.h"
#include "../Platform/System.h"
#include "../Timer.h"

#define NTP_PORT 123
#define NTP_PACKET_SIZE 48
#define NTP_VERSION 4
#define NTP_MODE_CLIENT 3
#define NTP_MODE_SERVER 4

#define NTP_SERVER_DEFAULT "pool.ntp.org"

#define NTP_LISTEN_PORT 57001
#define NTP_DEFAULT_AUTO_UPDATE_INTERVAL 600000 // 10 minutes


class NtpClient;
typedef void (*NtpTimeResultCallback)(NtpClient& client, uint32_t unixTime);


class NtpClient : protected UdpConnection
{
public:
	
	NtpClient(NtpTimeResultCallback onTimeReceivedCb);
	virtual ~NtpClient();

	void requestTime();
	
	void setNtpServer(String server);
	void setNtpServer(IPAddress adress);
	
	void setAutoQuery(bool autoQuery);
	void setAutoQueryInterval(int seconds);
		
protected:
	int resolveServer();
	void onReceive(pbuf *buf, IPAddress remoteIP, uint16_t remotePort);
	
protected: 
	String server;
	IPAddress serverAddress;
	NtpTimeResultCallback onCompleted;
		
	Timer autoUpdateTimer;
		
	static void staticDnsResponse(const char *name, struct ip_addr *ip, void *arg);		
};

#endif /* APP_NTPCLIENT_H_ */
