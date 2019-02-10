/**
 * Please, note, that in order to run this sample you should recompile Sming with ENABLE_SSL=1.
 * The following three commands should be enough:
 *
 * cd Sming/Sming
 * make clean
 * make ENABLE_SSL=1
 */

#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include "Network/HttpClient.h"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

HttpClient httpClient;

/* Debug SSL functions */
void displaySessionId(SSL* ssl)
{
	int i;
	const uint8_t* session_id = ssl_get_session_id(ssl);
	int sess_id_size = ssl_get_session_id_size(ssl);

	if(sess_id_size > 0) {
		debugf("-----BEGIN SSL SESSION PARAMETERS-----");
		for(i = 0; i < sess_id_size; i++) {
			m_printf("%02x", session_id[i]);
		}

		debugf("\n-----END SSL SESSION PARAMETERS-----");
	}
}

/**
 * Display what cipher we are using
 */
void displayCipher(SSL* ssl)
{
	m_printf("CIPHER is ");
	switch(ssl_get_cipher_id(ssl)) {
	case SSL_AES128_SHA:
		m_printf("AES128-SHA");
		break;

	case SSL_AES256_SHA:
		m_printf("AES256-SHA");
		break;

	case SSL_AES128_SHA256:
		m_printf("SSL_AES128_SHA256");
		break;

	case SSL_AES256_SHA256:
		m_printf("SSL_AES256_SHA256");
		break;

	default:
		m_printf("Unknown - %d", ssl_get_cipher_id(ssl));
		break;
	}

	m_printf("\n");
}

int onDownload(HttpConnection& connection, bool success)
{
	debugf("\n=========[ URL: %s ]============", connection.getRequest()->uri.toString().c_str());
	debugf("RemoteIP: %s", (char*)connection.getRemoteIp());
	debugf("Got response code: %d", connection.getResponseCode());
	debugf("Success: %d", success);
	if(connection.getRequest()->method != HTTP_HEAD) {
		debugf("Got content starting with: %s", connection.getResponseString().substring(0, 1000).c_str());
	}
	SSL* ssl = connection.getSsl();
	if(ssl) {
		const char* common_name = ssl_get_cert_dn(ssl, SSL_X509_CERT_COMMON_NAME);
		if(common_name) {
			debugf("Common Name:\t\t\t%s\n", common_name);
		}
		displayCipher(ssl);
		displaySessionId(ssl);
	}

	return 0; // return 0 on success in your callbacks
}

void setSslFingerprints(HttpRequest* request)
{
	// SSL validation: If you want to check the remote server certificate against a fingerprint,
	// you can also add the lines below
	// Note: SSL is not compiled by default. In our example we set the ENABLE_SSL directive to 1
	// (See: ../Makefile-user.mk )
	request->setSslOptions(SSL_SERVER_VERIFY_LATER);

	static const uint8_t sha1Fingerprint[] PROGMEM = {0xc5, 0xf9, 0xf0, 0x66, 0xc9, 0x0a, 0x21, 0x4a, 0xbc, 0x37,
													  0xae, 0x6c, 0x48, 0xcc, 0x97, 0xa5, 0xc3, 0x35, 0x16, 0xdc};

	static const uint8_t publicKeyFingerprint[] PROGMEM = {
		0x33, 0x47, 0xd1, 0x8a, 0xc8, 0x52, 0xd4, 0xd6, 0xd0, 0xa2, 0xcb, 0x3f, 0x4b, 0x54, 0x1f, 0x91,
		0x64, 0x94, 0xa0, 0x9c, 0xa1, 0xe2, 0xf2, 0x4c, 0x68, 0xae, 0xc5, 0x27, 0x1c, 0x60, 0x83, 0xad};

	SSLFingerprints fingerprints;

	/*
	 * The line below shows how to trust only a certificate in which the public key matches the SHA256 fingerprint.
	 * When google changes the private key that they use in their certificate the SHA256 fingerprint should not match any longer.
	 */
	fingerprints.setSha256_P(publicKeyFingerprint);

	/*
	 * The line below shows how to trust only a certificate that matches the SHA1 fingerprint.
	 * When google changes their certificate the SHA1 fingerprint should not match any longer.
	 */
	fingerprints.setSha1_P(sha1Fingerprint);

	// request->pinCertificate(fingerprints);
}

void connectOk(IPAddress ip, IPAddress mask, IPAddress gateway)
{
	// [ GET request: The example below shows how to make HTTP requests ]

	// First: The HttpRequest object contains all the data that needs to be sent
	//    	  to the remote server.

	HttpRequest* getRequest = new HttpRequest(URL("https://httpbin.org/get"));
	getRequest->setMethod(HTTP_GET); // << you may set the method. If not set the default HTTP_GET method will be used

	// Headers: if you need to set custom headers then you can do something like ...
	HttpHeaders headers;
	headers[HTTP_HEADER_USER_AGENT] = _F("HttpClient/Sming"); // Prefer use of enumerated type for standard field names
	headers[F("X-Powered-By")] = _F("Sming");				  // Use text for non-standard field names
	getRequest->setHeaders(headers);
	// ... or like
	getRequest->setHeader(F("X-Powered-By"), F("Sming"));

	// SSL validation and fingerprinting
	setSslFingerprints(getRequest);

	/*
	 * Notice: If we use SSL we need to set the SSL settings only for the first request
	 * 		   and all consecutive requests to the same host:port will try to reuse those settings
	 */

	// If we want to process the response we can do it by setting a onRequestCallback
	getRequest->onRequestComplete(onDownload);

	// Second: We have to send that request using our httpClient
	httpClient.send(getRequest);

	// [ POST request: the example below shows how to set a POST request with form data and files. ]
	FileStream* fileStream = new FileStream("5K.txt");

	HttpRequest* postRequest = new HttpRequest(URL("https://httpbin.org/post"));
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

	HttpRequest* putRequest = new HttpRequest(URL("https://httpbin.org/put"));
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
	//	responseBodyFile->open("file.name", eFO_CreateNewAlways | eFO_WriteOnly);
	//	putRequest->setResponseStream(responseBodyFile); // << the complete body will be stored on your file system
	// see the implementation of `bool HttpClient::downloadFile(const String& url, const String& saveFileName, ...` for details.

	httpClient.send(putRequest);
}

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
