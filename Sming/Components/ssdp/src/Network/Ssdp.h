/*
 * Ssdp.h
 *
 *  Created on: Oct 13, 2020
 *      Author: slavey
 */

#pragma once

#include <SmingCore.h>
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Stream/TemplateStream.h>
#include <Network/HttpServer.h>

#define SSDP_MULTICAST_ADDR 239, 255, 255, 250
#define SSDP_URI "/ssdp/description.xml"

class Ssdp : protected UdpConnection
{
public:
	using MessageHandlerDelegate = Delegate<void(const HttpHeaders& headers)>;

	/**
	 * The following SSDP settings are available in the default description.xml template
	 * 	- friendlyName
	 * 	- deviceType - for example unpnp:tea-pot
	 *  - presentationURL
	 *  - serialNumber
	 *  - modelName
	 *  - modelNumber
	 *  - modelURL
	 *  - manufacturer
	 *  - manufacturerURL
	 *  - UDN - unique device id
	 */
	using Settings = TemplateStream::Variables;

	Ssdp(uint16_t webPort = 8080);

	Ssdp(const Settings& settings, uint16_t webPort = 8080) : Ssdp(webPort)
	{
		setSettings(settings);
	}

	/**
	 * @brief This function is used to set the following SSDP variables
	 * 	- friendlyName
	 * 	- deviceType - for example unpnp:tea-pot
	 *  - presentationURL
	 *  - serialNumber
	 *  - modelName
	 *  - modelNumber
	 *  - modelURL
	 *  - manufacturer
	 *  - manufacturerURL
	 *  - UDN - unique device id
	 *
	 * @note  The function should be called before `connect`
	 */
	void setSettings(const Settings& settings)
	{
		this->settings.setMultiple(settings);
	}

	/**
	 * @brief This function is used to set the XML template
	 * @note  The function should be called before `connect`
	 */
	void setXmlTemplate(const String& xml)
	{
		xmlTemplate = xml;
	}

	/**
	 * @brief This function is used to register additional paths to the web server
	 * @note  The function should be called before `connect`
	 */
	void setWebPaths(const HttpResourceTree& paths)
	{
		for(size_t i = 0; i < paths.count(); i++) {
			const String key = paths.keyAt(i);
			server.paths[key] = (HttpResource*)paths[key];
		}
	}

	void setTtl(uint8_t ttl)
	{
		multicastTtl = ttl;
	}

	bool connect(IpAddress ip = IpAddress(239, 255, 255, 250), uint16_t port = 1900);

	void close() override;

	bool search(const String& uniqueId = nullptr);

	void setResponseHandler(MessageHandlerDelegate handler)
	{
		onResponse = handler;
	}

protected:
	virtual bool process(char* data, size_t length, IpAddress remoteIp, uint16_t remotePort);

	void onReceive(pbuf* buf, IpAddress remoteIp, uint16_t remotePort) override;

	void resetHeaders();

	/** @brief Called when all headers are received
	 * 	@param headers The processed headers
	 * 	@retval int 0 on success, non-0 on error
	 */
	virtual int onHeadersComplete(const HttpHeaders& headers, http_parser* parser);

	/** @brief Called when the incoming data is complete
	 * 	@param parser
	 * 	@retval int 0 on success, non-0 on error
	 */
	virtual int onMessageComplete(http_parser* parser);

	virtual bool sendFoundResponse();

	/**
	 * @brief Generates unique device id
	 */
	virtual String getUniqueUDN();

protected:
	// @TODO:  put the xmlTemplate in PROGMEM
	String xmlTemplate = "<?xml version=\"1.0\"?>"
						 "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
						 "<specVersion>"
						 "<major>1</major>"
						 "<minor>0</minor>"
						 "</specVersion>"
						 "<URLBase>{baseURL}</URLBase>"
						 "<device>"
						 "<deviceType>{deviceType}</deviceType>"
						 "<friendlyName>{friendlyName}</friendlyName>"
						 "<presentationURL>{presentationURL}</presentationURL>"
						 "<serialNumber>{serialNumber}</serialNumber>"
						 "<modelName>{modelName}</modelName>"
						 "<modelNumber>{modelNumber}</modelNumber>"
						 "<modelURL>{modelURL}</modelURL>"
						 "<manufacturer>{manufacturer}</manufacturer>"
						 "<manufacturerURL>{manufacturerURL}</manufacturerURL>"
						 "<UDN>{UDN}</UDN>"
						 "</device>"
						 "<iconList>"
						 "<icon>"
						 "<mimetype>image/png</mimetype>"
						 "<height>48</height>"
						 "<width>48</width>"
						 "<depth>24</depth>"
						 "<url>icon48.png</url>"
						 "</icon>"
						 "<icon>"
						 "<mimetype>image/png</mimetype>"
						 "<height>120</height>"
						 "<width>120</width>"
						 "<depth>24</depth>"
						 "<url>icon120.png</url>"
						 "</icon>"
						 "</iconList>"
						 "</root>\r\n"
						 "\r\n";

	http_parser parser;
	static const http_parser_settings parserSettings; ///< Callback table for parser

private:
	static int staticOnHeaderField(http_parser* parser, const char* at, size_t length);
	static int staticOnHeaderValue(http_parser* parser, const char* at, size_t length);
	static int staticOnHeadersComplete(http_parser* parser);
	static int staticOnMessageComplete(http_parser* parser);

private:
	IpAddress ip;	  /// < SSDP multicast IP
	uint16_t port = 0; /// < SSDP multicast port
	uint8_t multicastTtl = 2;

	HttpServer server;
	uint16_t webPort; // web server listening port

	Settings settings;

	MemoryDataStream* data = nullptr;
	String description;

	HttpHeaderBuilder header;	///< Header construction
	HttpHeaders incomingHeaders; ///< Full set of incoming headers
	MessageHandlerDelegate onResponse = nullptr;
	IpAddress remoteIp;		 /// < Remote M-SEARCH request IP
	uint16_t remotePort = 0; /// < Remote M-SEARCH request port
};
