#include <SmingCore.h>
extern "C" {
#include <lwip/app/ping.h>
}

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
ping_option pingOptions;
uint8_t failedAttempts;
constexpr uint8_t PING_COUNT = 3;
constexpr uint8_t MAX_FAILED_ATTEMTPS = 5;
constexpr uint8_t PING_INTERVAL_SECONDS = 10;
constexpr uint8_t RESTART_DELAY_SECONDS = 2;

Timer procTimer;

void ping(uint32_t ip);

void pingTask()
{
	ping(IpAddress("8.8.8.8")); // ping Google DNS servers
}

void onSent(void* arg, void* pdata)
{
	ping_resp* response = reinterpret_cast<ping_resp*>(pdata);

	if(response == nullptr) {
		debug_e("Invalid onSent call");
		return;
	}

	Serial << _F("Ping sent. Total failed attempts: ") << failedAttempts << endl;
	if(failedAttempts / response->total_count > MAX_FAILED_ATTEMTPS) {
		debug_d("Scheduling system restart in %d seconds.", RESTART_DELAY_SECONDS);
		// schedule restart
		System.restart(RESTART_DELAY_SECONDS * 1000);
		return;
	}

	debug_d("Scheduling another ping in %d seconds", PING_INTERVAL_SECONDS);
	procTimer.initializeMs<PING_INTERVAL_SECONDS * 1000>(pingTask).startOnce();
}

void onReceived(void* arg, void* pdata)
{
	ping_resp* response = reinterpret_cast<ping_resp*>(pdata);
	if(response == nullptr) {
		debug_e("Invalid onReceived call");
		return;
	}

	Serial << _F("Ping received. Sequence: ") << response->seqno << _F(", Success: ") << (response->ping_err == 0)
		   << _F(", Elapsed time: ") << response->total_time << endl;

	if(response->ping_err) {
		failedAttempts++;
	} else {
		failedAttempts = 0;
	}
}

void ping(uint32_t ip)
{
	debug_d("Ping IP: %s", IpAddress(ip).toString().c_str());
	pingOptions.ip = ip;
	pingOptions.count = PING_COUNT;
	pingOptions.recv_function = onReceived;
	pingOptions.sent_function = onSent;
	ping_start(&pingOptions);
}

void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	debug_d("Scheduling initial ping in 1 second.");
	procTimer.initializeMs<1000>(pingTask).startOnce();
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);

	WifiEvents.onStationGotIP(connectOk);
}
