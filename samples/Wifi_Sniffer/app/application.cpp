#include <SmingCore.h>

#include "Platform/WifiSniffer.h"
#include "Data/HexString.h"

namespace
{
BeaconInfoList knownAPs;	 ///< List of known APs
ClientInfoList knownClients; ///< List of known CLIENTs

const unsigned scanTimeoutMs = 2000; ///< End scan on channel if no new devices found within this time

WifiSniffer sniffer;
SimpleTimer timer;

/*
 * There may be non-printable characters in received SSID strings.
 * Replace these with ?.
 * Return a String of exactly 32 characters.
 */
String makeSsidString(const uint8_t* ssid, size_t len)
{
	String s;
	s.pad(32);
	len = std::min(len, s.length());
	std::transform(ssid, ssid + len, s.begin(), [](char c) { return isprint(c) ? c : '?'; });
	return s;
}

void printBeacon(const BeaconInfo& beacon)
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

void printClient(const ClientInfo& client)
{
	if(client.err != 0) {
		Serial << _F("CLIENT ERR: (") << client.err << ')' << endl;
		return;
	}

	Serial << _F("DEVICE: ") << client.station << _F(" ==> ");

	int i = knownAPs.indexOf(client.bssid);
	if(i < 0) {
		Serial << _F("Unknown/Malformed packet, BSSID = ") << client.bssid << endl;
		return;
	}

	auto& ap = knownAPs[i];
	String ssid = makeSsidString(ap.ssid, ap.ssid_len);
	Serial << '[' << ssid << ']' << "  " << client.ap << "  " << String(ap.channel).padLeft(3) << "   "
		   << String(client.rssi).padLeft(4) << endl;
}

void printSummary()
{
	DEFINE_FSTR_LOCAL(SEPARATOR,
					  "\r\n"
					  "-------------------------------------------------------------------------------------\r\n")

	Serial.println(SEPARATOR);
	for(auto& client : knownClients) {
		printClient(client);
	}
	for(auto& ap : knownAPs) {
		printBeacon(ap);
	}
	Serial.println(SEPARATOR);
}

void onBeacon(const BeaconInfo& beacon)
{
	if(knownAPs.indexOf(beacon.bssid) >= 0) {
		// Already listed
		return;
	}

	knownAPs.add(beacon);
	printBeacon(beacon);
	timer.restart();
}

void onClient(const ClientInfo& client)
{
	if(knownClients.indexOf(client.station) >= 0) {
		// Allready listed
		return;
	}

	knownClients.add(client);
	printClient(client);
	timer.restart();
}

void scanChannel(void* param)
{
	auto channel = reinterpret_cast<uint32_t>(param);
	if(channel <= 15) {
		// Scan the next channel
		debugf("Set channel: %u", channel);
		sniffer.setChannel(channel);
		timer.initializeMs<scanTimeoutMs>(scanChannel, reinterpret_cast<void*>(channel + 1));
		timer.startOnce();
		return;
	}

	// Stop sniffing and display final scan results
	sniffer.end();
	printSummary();
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	Serial << endl << endl << _F("SDK version:") << system_get_sdk_version() << endl;
	Serial.println(_F("ESP8266 mini-sniff by Ray Burnette http://www.hackster.io/rayburne/projects"));
	Serial.println(
		_F("Type:   /---------MAC---------/-----WiFi Access Point SSID-----/  /------MAC------/  Chnl  RSSI"));

	sniffer.onBeacon(onBeacon);
	sniffer.onClient(onClient);
	sniffer.begin();

	scanChannel(reinterpret_cast<void*>(1));
}
