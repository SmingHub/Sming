#include <SmingCore.h>
#include <Network/Http/Websocket/WebsocketResource.h>
#include <Network/Http/Resource/HttpAuth.h>
#include "ContentDecoderPlugin.h"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
HttpServer* server;

void echoContentBody(HttpRequest& request, HttpResponse& response)
{
	auto body = request.getBody();
	debug_d("Got content (after modifications): %s", body.c_str());

	response.headers[HTTP_HEADER_CONTENT_TYPE] = request.headers[HTTP_HEADER_CONTENT_TYPE];
	response.sendString(body);
}

void startWebServer()
{
	server = new HttpServer;

	server->listen(80);
	server->paths.set("/", echoContentBody);
	server->paths.setDefault(echoContentBody);

	/*
	 * By default the server does not store the incoming information.
	 * There has to be either a plugin that does this or a body parser.
	 * In this sample we use a body parser that stores the information into memory.
	 */
	server->setBodyParser(MIME_FORM_URL_ENCODED, bodyToStringParser);

	// Here we use a simple plugin that protects the access to a resource using HTTP Basic Authentication
	auto pluginBasicAuth = new ResourceBasicAuth("realm", "username", "password");
	// You can add one or more authentication methods or other plugins...
	server->paths.set("/auth", echoContentBody, pluginBasicAuth);

	/*
	 * The plugins will be registered in the order in which they are provided.
	 * For example in the command below the IP restriction plugin will be registered first
	 * followed by the basic authentication plugin.
	 * You can run the following curl command to test these plugins:
	 *
	 * 	curl -vvv http://username:password@192.168.13.10/ip-n-auth
	 *
	 * make sure to replace the IP address with the IP address of your HttpServer
	*/
	server->paths.set("/ip-n-auth", echoContentBody,
					  new ResourceIpAuth(IpAddress("192.168.13.0"), IpAddress("255.255.255.0")), pluginBasicAuth);

	/*
	 * This content coming to this resource is modified on the fly
	 * using our ContentDecoderPlugin. See the source code of ContentDecoderPlugin
	 * to get an idea how to create your own plugin.
	 * You can run the following curl command to test this plugin:
	 *
	 * 		curl -vvv http://username@192.168.13.10/test -d "1234" -H "Content-Encoding: test"
	 *
	 * 	make sure to replace the IP address with the IP address of your HttpServer
	 */
	server->paths.set("/test", echoContentBody, new ContentDecoderPlugin());

	Serial.println(F("\r\n=== WEB SERVER STARTED ==="));
	Serial.println(WifiStation.getIP());
	Serial.println(F("==============================\r\n"));
}

// Will be called when WiFi station becomes fully operational
void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	startWebServer();
	debug_i("free heap = %u", system_get_free_heap_size());
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiEvents.onStationGotIP(gotIP);
}
