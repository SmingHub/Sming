#include <user_config.h>
#include <SmingCore/SmingCore.h>

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

#define APP_ID		"55719abba4c48a802c881205"
#define APP_SECRET	"5300adbe3f906938950fc0cdbc301986"


// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

class InstapushTrackers : public HashMap<String, String>
{
};

class InstapushApplication : protected HttpClient
{
public:
	InstapushApplication(String appId, String appSecret)
	{
		app = appId;
		secret = appSecret;
	}

	void notify(String event, InstapushTrackers &trackersInfo)
	{
		debugf("preparing request");

		HttpRequest *request = new HttpRequest(URL(url));

		HttpHeaders requestHeaders;
		requestHeaders["Content-Type"] = "application/json";
		requestHeaders["x-instapush-appid"] =  app;
		requestHeaders["x-instapush-appsecret"] =  secret;

		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		root["event"] = event;
		JsonObject& trackers = root.createNestedObject("trackers");
		for (int i = 0; i < trackersInfo.count(); i++)
		{
			debugf("%s: %s", trackersInfo.keyAt(i).c_str(), trackersInfo.valueAt(i).c_str());
			trackers[trackersInfo.keyAt(i)] =  trackersInfo[trackersInfo.keyAt(i)];
		}

		String tempString;
		root.printTo(tempString);
		request->setBody(tempString);
		request->onRequestComplete(RequestCompletedDelegate(&InstapushApplication::processed, this));

		send(request);
	}

	int processed(HttpConnection& client, bool successful)
	{
		Serial.println(client.getResponseString());

		return 0;
	}

private:
	String app;
	String secret;
	const char *url = "http://api.instapush.im/v1/post";
};

Timer procTimer;
InstapushApplication pusher(APP_ID, APP_SECRET);

// Publish our message
void publishMessage()
{
	Serial.println("Push message now!");
	InstapushTrackers trackers;
	trackers["title"] = "Sming Framework";
	trackers["text"] = "New test was successfully launched";
	pusher.notify("notify", trackers); // event name, trackers
}

// Will be called when WiFi station was connected to AP
void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	Serial.println("I'm CONNECTED");

	// Start publishing loop
	procTimer.initializeMs(10 * 1000, publishMessage).start(true); // every 20 seconds
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
	WifiEvents.onStationGotIP(gotIP);
}
