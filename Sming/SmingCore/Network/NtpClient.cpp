/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 ****/

#include "NtpClient.h"
#include "Platform/Station.h"
#include "SystemClock.h"

NtpClient::NtpClient() : NtpClient(NTP_DEFAULT_SERVER, NTP_DEFAULT_QUERY_INTERVAL_SECONDS, nullptr)
{
}

NtpClient::NtpClient(NtpTimeResultDelegate onTimeReceivedCb)
	: NtpClient(NTP_DEFAULT_SERVER, NTP_DEFAULT_QUERY_INTERVAL_SECONDS, onTimeReceivedCb)
{
}

NtpClient::NtpClient(const String& reqServer, int reqIntervalSeconds, NtpTimeResultDelegate delegateFunction)
{
	// init timer but do not start, correct interval set later below.
	autoUpdateTimer.initializeMs(NTP_DEFAULT_QUERY_INTERVAL_SECONDS * 1000,
								 TimerDelegate(&NtpClient::requestTime, this));

	this->server = reqServer;
	this->delegateCompleted = delegateFunction;
	if(!delegateFunction) {
		autoUpdateSystemClock = true;
	}

	if(reqIntervalSeconds == 0) {
		setAutoQuery(false);
	} else {
		setAutoQueryInterval(reqIntervalSeconds);
		setAutoQuery(true);
		requestTime();
	}
}

void NtpClient::requestTime()
{
	if(!WifiStation.isConnected()) {
		connectionTimer.setCallback([](void* arg) { reinterpret_cast<NtpClient*>(arg)->requestTime(); }, this);
		connectionTimer.startMs(1000);
		return;
	}

	ip_addr_t resolvedIp;
	int result = dns_gethostbyname(server.c_str(), &resolvedIp,
								   [](const char* name, LWIP_IP_ADDR_T* ip, void* arg) {
									   // We do a new request since the last one was never done.
									   if(ip)
										   reinterpret_cast<NtpClient*>(arg)->internalRequestTime(*ip);
								   },
								   this);

	switch(result) {
	case ERR_OK:
		// Documentation says this will be the result if the string is already
		// an ip address in dotted decimal form or if the host is found in dns cache.
		// however I'm not sure if the dns cache is working since this never seems to
		// be called for a host lookup other than an ip address.
		// Doesn't really matter since the loockup will be fast anyways, the host
		// is most likely found in the dns cache of the next node the query is sent to.
		internalRequestTime(resolvedIp);
		break;
	case ERR_INPROGRESS:
		// currently finding ip, internalRequestTime() will be called when its found.
		//debug_d("DNS IP lookup in progress.");
		break;
	default:
		debug_d("DNS lookup error occurred.");
		break;
	}
}

void NtpClient::internalRequestTime(IPAddress serverIp)
{
	// connect to current active serverIp, on NTP_PORT
	connect(serverIp, NTP_PORT);

	uint8_t packet[NTP_PACKET_SIZE];

	// Setup the NTP request packet
	memset(packet, 0, NTP_PACKET_SIZE);

	// These are the only required values for a SNTP request. See page 14:
	// https://tools.ietf.org/html/rfc4330
	// However size of packet should still be 48 bytes.
	packet[0] = (NTP_VERSION << 3 | 0x03); // LI (0 = no warning), Protocol version (4), Client mode (3)
	packet[1] = 0;						   // Stratum, or type of clock, unspecified.

	// Start timeout timer, if no response is recieved within NTP_RESPONSE_TIMEOUT
	// a new request will be sent.
	timeoutTimer.setCallback([](void* arg) { reinterpret_cast<NtpClient*>(arg)->requestTime(); }, this);
	timeoutTimer.startMs(NTP_RESPONSE_TIMEOUT_MS);

	// Send to server, serverAddress & port is set in connect
	NtpClient::send((char*)packet, NTP_PACKET_SIZE);
}

void NtpClient::setAutoQueryInterval(int seconds)
{
	// minimum 10 seconds interval.
	if(seconds < 10)
		autoUpdateTimer.setIntervalMs(10000);
	else
		autoUpdateTimer.setIntervalMs(seconds * 1000);
}

void NtpClient::setAutoUpdateSystemClock(bool autoUpdateClock)
{
	autoUpdateSystemClock = autoUpdateClock;
}

void NtpClient::onReceive(pbuf* buf, IPAddress remoteIP, uint16_t remotePort)
{
	// stop timeout timer since we received a response.
	timeoutTimer.stop();

	// We do some basic check to see if it really is a ntp packet we receive.
	// NTP version should be set to same as we used to send, NTP_VERSION
	// NTP_VERSION 3 has time in same location so accept that too
	// Mode should be set to NTP_MODE_SERVER

	uint8_t versionMode = pbuf_get_at(buf, 0);
	uint8_t ver = (versionMode & 0b00111000) >> 3;
	uint8_t mode = (versionMode & 0x07);

	if(mode == NTP_MODE_SERVER && (ver == NTP_VERSION || ver == (NTP_VERSION - 1))) {
		//Most likely a correct NTP packet received.

		uint32_t timestamp;
		pbuf_copy_partial(buf, &timestamp, sizeof(timestamp), 40); // Copy only timestamp.
		timestamp = ntohl(timestamp);

		// Unix time starts on Jan 1 1970, subtract 70 years:
		uint32_t epoch = timestamp - 0x83AA7E80;

		if(autoUpdateSystemClock) {
			SystemClock.setTime(epoch, eTZ_UTC); // update systemclock utc value
		}

		if(delegateCompleted) {
			delegateCompleted(*this, epoch);
		}
	}
}
