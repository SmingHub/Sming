#include <SmingCore.h>
#include <JsonObjectStream.h>

/*** Direct PUSH notifications on your mobile phone!
 *
 * At first you should create new event in Instapush
 * 0. Go to http://instapush.im
 * 1. Create Application: "SmingTest"
 * 2. Add event:
 *    - Event Title: "notify"
 *    - Trackers: title [Tab] text
 *    - Push Message: "{title}! Details: {text}"
 * 3. Update Application ID and Application Secret below:
 */

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
DEFINE_FSTR(APP_ID, "55719abba4c48a802c881205")
DEFINE_FSTR(APP_SECRET, "5300adbe3f906938950fc0cdbc301986")

using InstapushTrackers = HashMap<String, String>;

class InstapushApplication : protected HttpClient
{
public:
	InstapushApplication(const String& appId, const String& appSecret) : app(appId), secret(appSecret)
	{
	}

	void notify(const String& event, InstapushTrackers& trackersInfo)
	{
		debugf("preparing request");

		HttpRequest* request = new HttpRequest(String(url));

		HttpHeaders requestHeaders;
		requestHeaders[HTTP_HEADER_CONTENT_TYPE] = toString(MIME_JSON);
		requestHeaders[F("x-instapush-appid")] = app;
		requestHeaders[F("x-instapush-appsecret")] = secret;

		auto stream = new JsonObjectStream(1024);
		auto root = stream->getRoot();
		root["event"] = event;
		JsonObject trackers = root.createNestedObject("trackers");
		for(auto info : trackersInfo) {
			debugf("%s: %s", info.key().c_str(), info.value().c_str());
			trackers[info.key()] = info.value();
		}

		request->setBody(stream);
		request->onRequestComplete(RequestCompletedDelegate(&InstapushApplication::processed, this));

		send(request);
	}

	int processed(HttpConnection& client, bool successful)
	{
		Serial.copyFrom(client.getResponse()->stream);

		return 0;
	}

private:
	String app;
	String secret;
	DEFINE_FSTR_LOCAL(url, "http://api.instapush.im/v1/post");
};

SimpleTimer procTimer;
InstapushApplication pusher(APP_ID, APP_SECRET);

// Publish our message
void publishMessage()
{
	Serial.println(_F("Push message now!"));
	InstapushTrackers trackers;
	trackers["title"] = F("Sming Framework");
	trackers["text"] = F("New test was successfully launched");
	pusher.notify("notify", trackers); // event name, trackers
}

// Will be called when WiFi station was connected to AP
void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial << _F("I'm CONNECTED to ") << ip << endl;

	// Start publishing loop
	procTimer.initializeMs<10 * 1000>(publishMessage).start();
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
	WifiEvents.onStationGotIP(gotIP);
}
