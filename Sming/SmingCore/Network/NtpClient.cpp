#include "NtpClient.h"

NtpClient::NtpClient(NtpTimeResultCallback onTimeReceivedCb)
{
	this->onCompleted = onTimeReceivedCb;
	this->server = NTP_SERVER_DEFAULT;
	autoUpdateTimer.initializeMs(NTP_DEFAULT_AUTO_UPDATE_INTERVAL, Delegate<void()>(&NtpClient::requestTime, this));
}

NtpClient::NtpClient()
{
	autoUpdateTimer.initializeMs(NTP_DEFAULT_AUTO_UPDATE_INTERVAL, Delegate<void()>(&NtpClient::requestTime, this));
	setAutoQuery(true);
	requestTime();
}

NtpClient::NtpClient(String reqServer, int reqIntervalSeconds, NtpTimeResultCallback onTimeReceivedCb)
// : NtpClient(reqServer, reqIntervalSeconds, NtpClientResultCallback (onTimeReceivedCb))
{
	autoUpdateTimer.initializeMs(NTP_DEFAULT_AUTO_UPDATE_INTERVAL, Delegate<void()>(&NtpClient::requestTime, this));
	this->server = reqServer;
	this->onCompleted = onTimeReceivedCb;
	if (!onTimeReceivedCb)
	{
		autoUpdateSystemClock = true;
	}
	if (reqIntervalSeconds == 0)
	{
		setAutoQuery(false);
	}
	else
	{
		setAutoQueryInterval(reqIntervalSeconds);
		setAutoQuery(true);
		requestTime();
	}
}

NtpClient::NtpClient(String reqServer, int reqIntervalSeconds, NtpClientResultCallback delegateFunction /* = NULL */)
{
	autoUpdateTimer.initializeMs(NTP_DEFAULT_AUTO_UPDATE_INTERVAL, Delegate<void()>(&NtpClient::requestTime, this));
	this->server = reqServer;
	this->delegateCompleted = delegateFunction;
	if (!delegateFunction)
	{
		autoUpdateSystemClock = true;
	}

	if (reqIntervalSeconds == 0)
	{
		setAutoQuery(false);
	}
	else
	{
		setAutoQueryInterval(reqIntervalSeconds);
		setAutoQuery(true);
		requestTime();
	}

}

NtpClient::~NtpClient()
{
}

int NtpClient::resolveServer()
{
	if (this->server != NULL)
	{
		struct ip_addr resolved;
		switch (dns_gethostbyname(this->server.c_str(), &resolved,
				staticDnsResponse, (void*) this))
		{
		case ERR_OK:
			serverAddress = resolved;
			return 1;
			break;
		case ERR_INPROGRESS:
			return 0; // currently finding ip, requestTime() will be called later.
		default:
			debugf("DNS Lookup error occurred.");
			return 0;
		}
	}

	return 0;
}

void NtpClient::requestTime()
{
	debugf("NtpClient request Time");
	if (!WifiStation.isConnected())
	{
		connectionTimer.initializeMs(1000, Delegate<void()>(&NtpClient::requestTime, this)).startOnce();
		return;
	}
	if (serverAddress.isNull())
	{
		if (!resolveServer())
		{
			return;
		}
	}

//	connect to current active serverAddress, on NTP_PORT
	this->connect(serverAddress,NTP_PORT);

	uint8_t packet[NTP_PACKET_SIZE];

	// Setup the NTP request packet
	memset(packet, 0, NTP_PACKET_SIZE);

	// These are the only required values for a SNTP request. See page 14:
	// https://tools.ietf.org/html/rfc4330 
	// However size of packet should still be 48 bytes.
	packet[0] = (NTP_VERSION << 3 | 0x03); // LI (0 = no warning), Protocol version (4), Client mode (3)
	packet[1] = 0;     	// Stratum, or type of clock, unspecified.

//	Send to server, serverAddress & port is set in connect
	NtpClient::send((char*) packet, NTP_PACKET_SIZE);
	debugf("NtpClient request sent");
}

void NtpClient::setNtpServer(String server)
{
	this->server = server;
	// force new DNS lookup
	serverAddress = (uint32_t)0;
}

void NtpClient::setNtpServer(IPAddress serverIp)
{
	this->server = NULL;
	this->serverAddress = serverIp;
}

void NtpClient::setAutoQuery(bool autoQuery)
{
	if (autoQuery)
		autoUpdateTimer.start();
	else
		autoUpdateTimer.stop();
}

void NtpClient::setAutoQueryInterval(int seconds)
{
	// minimum 10 seconds interval.
	if (seconds < 10)
		autoUpdateTimer.setIntervalMs(10000);
	else
		autoUpdateTimer.setIntervalMs(seconds * 1000);
}

void NtpClient::setAutoUpdateSystemClock(bool autoUpdateClock)
{
	autoUpdateSystemClock = autoUpdateClock;
}

void NtpClient::onReceive(pbuf *buf, IPAddress remoteIP, uint16_t remotePort)
{
	// We do some basic check to see if it really is a ntp packet we receive.
	// NTP version should be set to same as we used to send, NTP_VERSION
	// NTP_VERSION 3 has time in same location so accept that too
	// Mode should be set to NTP_MODE_SERVER

	debugf("NtpClient onReceive");

	uint8_t versionMode = pbuf_get_at(buf, 0);
	uint8_t ver = (versionMode & 0b00111000) >> 3;
	uint8_t mode = (versionMode & 0x07);

	if (mode == NTP_MODE_SERVER && (ver == NTP_VERSION || ver == (NTP_VERSION -1)))
	{
		//Most likely a correct NTP packet received.

		uint8_t data[4];
		pbuf_copy_partial(buf, data, 4, 40); // Copy only timestamp.

		uint32_t timestamp = (data[0] << 24 | data[1] << 16 | data[2] << 8
				| data[3]);

		// Unix time starts on Jan 1 1970, subtract 70 years:
		uint32_t epoch = timestamp - 0x83AA7E80;

		if (autoUpdateSystemClock)
		{
			debugf("NtpClient onreceive autoupdate");
			SystemClock.setTime(epoch, true); // update systemclock utc value
		}

		if (onCompleted != NULL)
		{
			debugf("NtpClient onreceive oncompleted");
			this->onCompleted(*this, epoch);
		}
		if (delegateCompleted)
		{
			debugf("NtpClient onreceive delegated");
			this->delegateCompleted(*this, epoch);
		}
	}
}

void NtpClient::staticDnsResponse(const char *name, struct ip_addr *ip, void *arg)
{
	// DNS has been resolved

	NtpClient *self = (NtpClient*) arg;

	if (ip != NULL)
	{
		self->serverAddress = *ip;
		// We do a new request since the last one was never done.
		self->requestTime();
	}
}
