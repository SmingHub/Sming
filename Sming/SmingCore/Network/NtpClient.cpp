#include "NtpClient.h"

NtpClient::NtpClient(NtpTimeResultCallback onTimeReceivedCb)
{
	this->onCompleted = onTimeReceivedCb;
	this->autoUpdate = false;
	this->autoUpdateInteval = NTP_DEFAULT_AUTO_UPDATE_INTERVAL;
	this->server = NTP_SERVER_DEFAULT;
}

NtpClient::~NtpClient()
{
}

int NtpClient::init()
{
	this->listen(NTP_LISTEN_PORT);

	struct ip_addr resolved;
	switch (dns_gethostbyname(NTP_SERVER_DEFAULT, &resolved, staticDnsResponse,
			(void*) this))
	{
	case ERR_OK:
		serverAddress = resolved;
		break;
	case ERR_INPROGRESS:
		return 0; // currently finding ip, requestTime() will be called later.
	default:
		debugf("DNS Lookup error occurred.");
		return 0;
	}

	return 1;
}

void NtpClient::requestTime()
{
	if (serverAddress.isNull())
	{
		if (!init())
		{
			return;
		}
	}

	uint8_t packet[NTP_PACKET_SIZE];

	// Setup the NTP request packet
	memset(packet, 0, NTP_PACKET_SIZE);

	// These are the only required values for a SNTP request. See page 14:
	// https://tools.ietf.org/html/rfc4330 
	// However size of packet should still be 48 bytes.
	packet[0] = (NTP_VERSION << 3 | 0x03); // LI (0 = no warning), Protocol version (4), Client mode (3)
	packet[1] = 0;     	// Stratum, or type of clock, unspecified.

	NtpClient::sendTo(serverAddress, NTP_PORT, (char*) packet, NTP_PACKET_SIZE);
}

void NtpClient::setNtpServer(String server) 
{
	server = server;
	// force new DNS lookup
	serverAddress = (uint32_t)0;
}

void NtpClient::setAutoQuery(bool autoQuery)
{
	if (autoQuery)
	{
		this->autoUpdate = true;
		ets_timer_disarm(&autoUpdateTimer);
		ets_timer_setfn(&autoUpdateTimer,
				(os_timer_func_t *) staticAutoUpdateCallback, this);
		ets_timer_arm_new(&autoUpdateTimer, autoUpdateInteval, true, 1);
	}
	else
	{
		ets_timer_disarm(&autoUpdateTimer);
		this->autoUpdate = false;
	}
}

void NtpClient::setAutoQueryInterval(int seconds)
{
	// minimum 10 seconds interval.
	if (seconds < 10)
		this->autoUpdateInteval = 10000;
	else
		this->autoUpdateInteval = seconds * 1000;

	// Restart timer since interval has changed.
	setAutoQuery(this->autoUpdate);
}

void NtpClient::onReceive(pbuf *buf, IPAddress remoteIP, uint16_t remotePort)
{
	// We do some basic check to see if it really is a ntp packet we receive.
	// NTP version should be set to same as we used to send, NTP_VERSION
	// Mode should be set to NTP_MODE_SERVER

	if (onCompleted != NULL)
	{

		uint8_t versionMode = pbuf_get_at(buf, 0);
		uint8_t ver = (versionMode & 0b00111000) >> 3;
		uint8_t mode = (versionMode & 0x07);

		if (mode == NTP_MODE_SERVER && ver == NTP_VERSION)
		{
			//Most likely a correct NTP packet received.

			uint8_t data[4];
			pbuf_copy_partial(buf, data, 4, 40); // Copy only timestamp.

			uint32_t timestamp = (data[0] << 24 | data[1] << 16 | data[2] << 8
					| data[3]);

			// Unix time starts on Jan 1 1970, subtract 70 years:
			uint32_t epoch = timestamp - 0x83AA7E80;

			this->onCompleted(*this, epoch);
		}
	}
}

void NtpClient::staticDnsResponse(const char *name, struct ip_addr *ip,
		void *arg)
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

void NtpClient::staticAutoUpdateCallback(void *arg)
{
	NtpClient *self = (NtpClient*) arg;

	// Request a time update.
	self->requestTime();
}
