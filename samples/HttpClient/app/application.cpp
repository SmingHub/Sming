#include <SmingCore.h>

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
	debugf("RemoteIP: %s", connection.getRemoteIp().toString().c_str());
	debugf("Got response code: %d", connection.getResponse()->code);
	debugf("Success: %d", success);
	if(connection.getRequest()->method != HTTP_HEAD) {
		debugf("Got content starting with: %s", connection.getResponse()->getBody().substring(0, 1000).c_str());
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
	/*
	 * SSL validation: We check the remote server certificate against a fingerprint
	 * Note that fingerprints _may_ change, in which case these need to be updated.
	 *
	 * Note: SSL is not compiled by default. In our example we set the ENABLE_SSL directive to 1
	 * (See: ../Makefile-user.mk )
	 */
	request->setSslOptions(SSL_SERVER_VERIFY_LATER);

	// These are the fingerprints for httpbin.org
	static const uint8_t sha1Fingerprint[] PROGMEM = {0x2B, 0xF0, 0x48, 0x9D, 0x78, 0xB4, 0xDE, 0xE9, 0x69, 0xE2,
													  0x73, 0xE0, 0x14, 0xD0, 0xDC, 0xCC, 0xA8, 0xD8, 0x3B, 0x40};

	static const uint8_t publicKeyFingerprint[] PROGMEM = {
		0xE3, 0x88, 0xC4, 0x0A, 0x2A, 0x99, 0x8F, 0xA4, 0x8C, 0x38, 0x4E, 0xE7, 0xCB, 0x4F, 0x8B, 0x99,
		0x19, 0x48, 0x63, 0x9A, 0x2E, 0xD6, 0x05, 0x7D, 0xB1, 0xD3, 0x56, 0x6C, 0xC0, 0x7E, 0x74, 0x1A};

	SslFingerprints fingerprints;

	// Trust only a certificate in which the public key matches the SHA256 fingerprint...
	fingerprints.setSha256_P(publicKeyFingerprint, sizeof(publicKeyFingerprint));

	// ... or a certificate that matches the SHA1 fingerprint.
	fingerprints.setSha1_P(sha1Fingerprint, sizeof(sha1Fingerprint));

	// Attached fingerprints to request for validation
	request->pinCertificate(fingerprints);
}

void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
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
