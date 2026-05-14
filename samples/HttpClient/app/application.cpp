#include <SmingCore.h>
#include <Network/HttpClient.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
HttpClient httpClient;
SimpleTimer requestTimer;

int onDownload(HttpConnection& connection, bool success)
{
	auto& request = *connection.getRequest();
	auto& response = *connection.getResponse();

	Serial << endl << _F("=========[ URL: ") << request.uri << _F(" ]============") << endl;
	Serial << _F("RemoteIP: ") << connection.getRemoteIp() << endl;
	Serial << _F("Got response code: ") << response.code << " " << toString(response.code) << endl;
	Serial << _F("Success: ") << success << " " << (success ? "OK" : "FAIL") << endl;

	if(request.method != HTTP_HEAD) {
		Serial.print(_F("Got content: "));
		auto stream = response.stream;
		if(stream == nullptr || stream->available() == 0) {
			Serial.println(_F("EMPTY!"));
		} else {
			Serial.copyFrom(stream);
			Serial.println();
		}
	}

	auto ssl = connection.getSsl();
	if(ssl != nullptr) {
		ssl->printTo(Serial);
	}

	return 0; // return 0 on success in your callbacks
}

void sslRequestInit(Ssl::Session& session, HttpRequest& request)
{
	// Go with maximum buffer sizes
	session.maxBufferSize = Ssl::MaxBufferSize::K16;

	/**
	 * If there is not enough RAM and the response from the server is not a big one
	 * you may prefer to set the size to a lower value. One useful value would be 4K
	 *
	 * 	session.maxBufferSize = Ssl::MaxBufferSize::K4;
	 *
	 * Unfortunately most of the servers may ignore this plea from
	 * our HTTP client and send big SSL packet.
	 */

	/*
	 * SSL validation: We check the remote server certificate against a fingerprint
	 * Note that fingerprints _may_ change, in which case these need to be updated.
	 *
	 * Note: SSL is not compiled by default. In our example we set the ENABLE_SSL directive to 1
	 * (See: ../component.mk)
	 */
	session.options.verifyLater = true;

	// These are the fingerprints for httpbin.org
	static const Ssl::Fingerprint::Cert::Sha1 sha1Fingerprint PROGMEM{
		0xDC, 0xF3, 0xDC, 0x23, 0xFC, 0xEB, 0xCE, 0x46, 0xDD, 0x40,
		0x91, 0xF4, 0x6A, 0x62, 0x00, 0x61, 0x3B, 0x6B, 0x05, 0xE6,
	};

	static const Ssl::Fingerprint::Pki::Sha256 publicKeyFingerprint PROGMEM{
		0x10, 0xE5, 0xC3, 0xBD, 0x42, 0x72, 0x0A, 0x3E, 0x3A, 0xEB, 0xE1, 0x4D, 0x04, 0x76, 0xE7, 0xF9,
		0x06, 0x44, 0x9A, 0x09, 0xF7, 0xC6, 0x89, 0x12, 0x66, 0x19, 0xAE, 0xEC, 0x57, 0x01, 0xF0, 0x45,

	};

	// Trust only a certificate in which the public key matches the SHA256 fingerprint...
	session.validators.pin(publicKeyFingerprint);

	// ... or a certificate that matches the SHA1 fingerprint.
	session.validators.pin(sha1Fingerprint);
}

void postMessage()
{
	Serial << endl << endl << _F("Posting message...") << endl;

	// [ GET request: The example below shows how to make HTTP requests ]

	// First: The HttpRequest object contains all the data that needs to be sent
	//    	  to the remote server.

	HttpRequest* getRequest = new HttpRequest(F("https://httpbin.org/get"));
	getRequest->setMethod(HTTP_GET); // << you may set the method. If not set the default HTTP_GET method will be used

	// Headers: if you need to set custom headers then you can do something like ...
	HttpHeaders headers;
	headers[HTTP_HEADER_USER_AGENT] = _F("HttpClient/Sming"); // Prefer use of enumerated type for standard field names
	headers[F("X-Powered-By")] = _F("Sming");				  // Use text for non-standard field names
	getRequest->setHeaders(headers);
	// ... or like
	getRequest->setHeader(F("X-Powered-By"), F("Sming"));

	/*
	 * Notice: If we use SSL we need to set the SSL settings only for the first request
	 * 		   and all consecutive requests to the same host:port will try to reuse those settings
	 */
	getRequest->onSslInit(sslRequestInit);

	// If we want to process the response we can do it by setting a onRequestCallback
	getRequest->onRequestComplete(onDownload);

	// Second: We have to send that request using our httpClient
	httpClient.send(getRequest);

	// [ POST request: the example below shows how to set a POST request with form data and files. ]
	FileStream* fileStream = new FileStream("5K.txt");

	HttpRequest* postRequest = new HttpRequest(F("https://httpbin.org/post"));
	// For this request we will use a slightly improved syntax
	postRequest
		->setMethod(HTTP_POST)					  // << we set the method to POST
		->setHeaders(headers)					  // << we add extra headers
		->setPostParameter("text", "Test upload") // << we set one form element called "text"
		->setFile("file1", fileStream)			  // << we set one file upload that should upload the data.txt
												  // ... under the form element name "file1"
		->onRequestComplete(onDownload);

	httpClient.send(postRequest); // << don't forget to `send` the request

	// [PUT request with raw data: We will send the data.txt content without any additional content encoding ]
	FileStream* fileStream1 = new FileStream("20K.txt");

	HttpRequest* putRequest = new HttpRequest(F("https://httpbin.org/put"));
	putRequest->setMethod(HTTP_PUT)
		->setBody(fileStream1) // << we set the complete HTTP body
		->onRequestComplete(onDownload);

	// Remark: Response body handling.
	// The remote server will "echo" the data meaning that the body will be bigger than 20K.
	// By default the http client will store in memory up to 1024 bytes.
	// If you want to store in memory larger body you can do something like this
	//	putRequest->setResponseStream(new LimitedMemoryStream(4096)); // << stores max 4K in memory

	// Or if you want to directly save the response body to a file then the following can be done
	//	FileStream* responseBodyFile = new FileStream();
	//	responseBodyFile->open("file.name", File::CreateNewAlways | File::WriteOnly);
	//	putRequest->setResponseStream(responseBodyFile); // << the complete body will be stored on your file system
	// see the implementation of `bool HttpClient::downloadFile(const String& url, const String& saveFileName, ...` for details.

	httpClient.send(putRequest);
}

void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial << _F("Connected. Got IP: ") << ip << endl;

	requestTimer.initializeMs<30000>(postMessage).start();
	postMessage();
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	// This sample is using files and we need to mount beforehand the file system
	spiffs_mount();

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);

	WifiEvents.onStationGotIP(connectOk);
}
