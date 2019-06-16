#include <SmingCore.h>

#include "Platform/WifiSniffer.h"
#include "Data/HexString.h"

static BeaconInfoList knownAPs;		///< List of known APs
static ClientInfoList knownClients; ///< List of known CLIENTs

const unsigned scanTimeoutMs = 2000; ///< End scan on channel if no new devices found within this time

WifiSniffer sniffer;
SimpleTimer timer;

static void restartTimer()
{
	timer.startMs(scanTimeoutMs, false);
}

static void printBeacon(const BeaconInfo& beacon)
{
	if(beacon.err != 0) {
		Serial.printf(_F("BEACON ERR: (%d)\n"), beacon.err);
	} else {
		Serial.printf(_F("BEACON: <=============== [%32s]  "), beacon.ssid);
		Serial.print(makeHexString(beacon.bssid, sizeof(beacon.bssid)));
		Serial.printf(_F("   %2d"), beacon.channel);
		Serial.printf(_F("   %4d\n"), beacon.rssi);
	}
}

static void printClient(const ClientInfo& client)
{
	if(client.err != 0) {
		Serial.printf(_F("CLIENT ERR: (%d)\n"), client.err);
	} else {
		Serial.print(_F("DEVICE: "));
		Serial.print(makeHexString(client.station, sizeof(client.station)));
		Serial.print(_F(" ==> "));

		int ap = knownAPs.indexOf(client.bssid);
		if(ap < 0) {
			Serial.print(_F("   Unknown/Malformed packet, BSSID = "));
			Serial.println(makeHexString(client.bssid, sizeof(client.bssid)));
		} else {
			Serial.printf(_F("[%32s]"), knownAPs[ap].ssid);
			Serial.print(_F("  "));
			Serial.print(makeHexString(client.ap, sizeof(client.ap)));
			Serial.printf(_F("  %3i"), knownAPs[ap].channel);
			Serial.printf(_F("   %4d\n"), client.rssi);
		}
	}
}

static void printSummary()
{
	Serial.println("\n-------------------------------------------------------------------------------------\n");
	for(unsigned u = 0; u < knownClients.count(); u++) {
		printClient(knownClients[u]);
	}
	for(unsigned u = 0; u < knownAPs.count(); u++) {
		printBeacon(knownAPs[u]);
	}
	Serial.println("\n-------------------------------------------------------------------------------------\n");
}

static void onBeacon(const BeaconInfo& beacon)
{
	if(knownAPs.indexOf(beacon.bssid) < 0) {
		knownAPs.add(beacon);
		printBeacon(beacon);
		restartTimer();
	}
}

static void onClient(const ClientInfo& client)
{
	if(knownClients.indexOf(client.station) < 0) {
		knownClients.add(client);
		printClient(client);
		restartTimer();
	}
}

static void scanChannel(void* param)
{
	auto channel = reinterpret_cast<uint32_t>(param);
	if(channel <= 15) {
		// Scan the next channel
		debugf("Set channel: %u", channel);
		sniffer.setChannel(channel);
		timer.setCallback(scanChannel, reinterpret_cast<void*>(channel + 1));
		restartTimer();
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

	Serial.printf(_F("\n\nSDK version:%s\n"), system_get_sdk_version());
	Serial.println(_F("ESP8266 mini-sniff by Ray Burnette http://www.hackster.io/rayburne/projects"));
	Serial.println(_F("Type:   /-------MAC------/-----WiFi Access Point SSID-----/  /----MAC---/  Chnl  RSSI"));

	sniffer.onBeacon(onBeacon);
	sniffer.onClient(onClient);
	sniffer.begin();

	scanChannel(reinterpret_cast<void*>(1));
}
