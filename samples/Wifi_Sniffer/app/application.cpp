#include <SmingCore.h>

#include "Platform/WifiSniffer.h"
#include "Data/HexString.h"

static BeaconInfoList knownAPs;		///< List of known APs
static ClientInfoList knownClients; ///< List of known CLIENTs

const unsigned scanTimeoutMs = 2000; ///< End scan on channel if no new devices found within this time

WifiSniffer sniffer;
SimpleTimer timer;

static void printBeacon(const BeaconInfo& beacon)
{
	if(beacon.err != 0) {
		Serial << _F("BEACON ERR: (") << beacon.err << ')' << endl;
	} else {
		Serial.printf(_F("BEACON: <=============== [%32s]  "), beacon.ssid);
		Serial.print(beacon.bssid);
		Serial.printf(_F("   %2d"), beacon.channel);
		Serial.printf(_F("   %4d\r\n"), beacon.rssi);
	}
}

static void printClient(const ClientInfo& client)
{
	if(client.err != 0) {
		Serial << _F("CLIENT ERR: (") << client.err << ')' << endl;
	} else {
		Serial << _F("DEVICE: ") << client.station << _F(" ==> ");

		int ap = knownAPs.indexOf(client.bssid);
		if(ap < 0) {
			Serial << _F("   Unknown/Malformed packet, BSSID = ") << client.bssid << endl;
		} else {
			Serial.printf(_F("[%32s]"), knownAPs[ap].ssid);
			Serial << "  " << client.ap;
			Serial.printf(_F("  %3i"), knownAPs[ap].channel);
			Serial.printf(_F("   %4d\r\n"), client.rssi);
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
	Serial.println(_F("Type:   /-------MAC------/-----WiFi Access Point SSID-----/  /----MAC---/  Chnl  RSSI"));

	sniffer.onBeacon(onBeacon);
	sniffer.onClient(onClient);
	sniffer.begin();

	scanChannel(reinterpret_cast<void*>(1));
}
