#include <SmingCore.h>

#include "Platform/WifiSniffer.h"
#include "Data/HexString.h"

static BeaconInfoList knownAPs;		///< List of known APs
static ClientInfoList knownClients; ///< List of known CLIENTs

const unsigned scanTimeoutMs = 2000; ///< End scan on channel if no new devices found within this time

WifiSniffer sniffer;
SimpleTimer timer;

/*
 * There may be non-printable characters in received SSID strings.
 * Replace these with ?.
 * Return a String of exactly 32 characters.
 */
static String makeSsidString(const uint8_t* ssid, size_t len)
{
	String s;
	s.pad(32);
	len = std::min(len, s.length());
	std::transform(ssid, ssid + len, s.begin(), [](char c) { return isprint(c) ? c : '?'; });
	return s;
}

static void printBeacon(const BeaconInfo& beacon)
{
	if(beacon.err != 0) {
		Serial << _F("BEACON ERR: (") << beacon.err << ')' << endl;
	} else {
		String ssid = makeSsidString(beacon.ssid, beacon.ssid_len);
		Serial << _F("BEACON: <==================== [") << ssid << "]  " << beacon.bssid << "  "
			   << String(beacon.channel).pad(2) << "  " << String(beacon.rssi).padLeft(4) << endl;
	}
	Serial << makeHexString(beacon.ssid, 32) << " " << beacon.ssid_len << endl;
}

static void printClient(const ClientInfo& client)
{
	if(client.err != 0) {
		Serial << _F("CLIENT ERR: (") << client.err << ')' << endl;
	} else {
		Serial << _F("DEVICE: ") << client.station << _F(" ==> ");

		int i = knownAPs.indexOf(client.bssid);
		if(i < 0) {
			Serial << _F("Unknown/Malformed packet, BSSID = ") << client.bssid << endl;
		} else {
			auto& ap = knownAPs[i];
			String ssid = makeSsidString(ap.ssid, ap.ssid_len);
			Serial << '[' << ssid << ']' << "  " << client.ap << "  " << String(ap.channel).padLeft(3) << "   "
				   << String(client.rssi).padLeft(4) << endl;
		}
	}
}

static void printSummary()
{
	Serial.println("\r\n"
				   "-------------------------------------------------------------------------------------\r\n");
	for(unsigned u = 0; u < knownClients.count(); u++) {
		printClient(knownClients[u]);
	}
	for(unsigned u = 0; u < knownAPs.count(); u++) {
		printBeacon(knownAPs[u]);
	}
	Serial.println("\r\n"
				   "-------------------------------------------------------------------------------------\r\n");
}

static void onBeacon(const BeaconInfo& beacon)
{
	if(knownAPs.indexOf(beacon.bssid) < 0) {
		knownAPs.add(beacon);
		printBeacon(beacon);
		timer.restart();
	}
}

static void onClient(const ClientInfo& client)
{
	if(knownClients.indexOf(client.station) < 0) {
		knownClients.add(client);
		printClient(client);
		timer.restart();
	}
}

static void scanChannel(void* param)
{
	auto channel = reinterpret_cast<uint32_t>(param);
	if(channel <= 15) {
		// Scan the next channel
		debugf("Set channel: %u", channel);
		sniffer.setChannel(channel);
		timer.initializeMs<scanTimeoutMs>(scanChannel, reinterpret_cast<void*>(channel + 1));
		timer.startOnce();
	} else {
		// Stop sniffing and display final scan results
		sniffer.end();
		printSummary();
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	Serial << _F("\r\n\r\n"
				 "SDK version:")
		   << system_get_sdk_version() << endl;
	Serial.println(_F("ESP8266 mini-sniff by Ray Burnette http://www.hackster.io/rayburne/projects"));
	Serial.println(
		_F("Type:   /---------MAC---------/-----WiFi Access Point SSID-----/  /------MAC------/  Chnl  RSSI"));

	sniffer.onBeacon(onBeacon);
	sniffer.onClient(onClient);
	sniffer.begin();

	scanChannel(reinterpret_cast<void*>(1));
}
