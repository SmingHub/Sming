#include "Ssdp.h"
#include <lwip/igmp.h>

/** @brief http_parser function table
 *  @note stored in flash memory; as it is word-aligned it can be accessed directly
 *  Notification callbacks: on_message_begin, on_headers_complete, on_message_complete
 *  Data callbacks: on_url, (common) on_header_field, on_header_value, on_body
 */
const http_parser_settings Ssdp::parserSettings PROGMEM = {
	.on_message_begin = nullptr,
	.on_url = nullptr,
	.on_status = nullptr,
	.on_header_field = staticOnHeaderField,
	.on_header_value = staticOnHeaderValue,
	.on_headers_complete = staticOnHeadersComplete,
	.on_body = nullptr,
	.on_message_complete = staticOnMessageComplete,
	.on_chunk_header = nullptr,
	.on_chunk_complete = nullptr,
};

#define GET_CONNECTION()                                                                                               \
	auto connection = static_cast<Ssdp*>(parser->data);                                                                \
	if(connection == nullptr) {                                                                                        \
		return -1;                                                                                                     \
	}

Ssdp::Ssdp(uint16_t webPort) : webPort(webPort)
{
	// default settings
	settings["baseURL"] = "http://" + WifiStation.getIP().toString() + ":" + webPort + SSDP_URI;
	settings["deviceType"] = "urn:sming-org:device:TestController:1";
	settings["friendlyName"] = "Sming Embedded Device";
	settings["manufacturer"] = "Sming Framework";
	settings["manufacturerURL"] = "https://github.com/SmingHub/Sming";
	settings["modelName"] = "Sming Smart Model";
	settings["modelNumber"] = "S01";
	settings["UDN"] = "uuid:" + String("<unique-id>"); // TODO: generate unique device id

	http_parser_init(&parser, HTTP_BOTH);
	parser.data = this;
}

bool Ssdp::connect(IpAddress ip, uint16_t port)
{
	server.listen(webPort);
	server.paths.set(SSDP_URI, [this](HttpRequest& request, HttpResponse& response) {
		response.setAllowCrossDomainOrigin("*");
		response.headers[HTTP_HEADER_CONNECTION] = F("close");

		MemoryDataStream* data = new MemoryDataStream();
		data->write((const uint8_t*)this->xmlTemplate.c_str(), this->xmlTemplate.length());
		TemplateStream* tmpl = new TemplateStream(data);
		tmpl->onGetValue([](const char* name) -> String { return ""; });
		tmpl->setVars(settings);

		response.sendDataStream(tmpl, MIME_XML);
	});

	debug_d("=== WEB SERVER STARTED === %s", WifiStation.getIP());

	IpAddress localIp = WifiStation.getIP();
	IpAddress multicastIp(SSDP_MULTICAST_ADDR);

	if(igmp_joingroup(localIp, multicastIp) != ERR_OK) {
		debug_e("SSDP failed to join igmp group\n"); // will be able to search but won't be discovered
	}

	UdpConnection::listen(port);

	UdpConnection::setMulticast(localIp);
	UdpConnection::setMulticastTtl(multicastTtl);

	this->ip = ip;
	this->port = port;
	HttpHeaders headers;
	headers["NTS"] = "ssdp:alive";
	headers["Cache-Control"] = "max-age=1800";
	headers["Location"] = settings["baseURL"];

	Url url;
	url.Host = ip.toString();
	url.Port = port;
	url.Path = "*";

	HttpRequest request(url);
	request.setMethod(HTTP_NOTIFY);
	request.setHeaders(headers);

	return sendStringTo(ip, port, request.toString());
}

void Ssdp::close()
{
	IpAddress localIp = WifiStation.getIP();
	IpAddress multicastIp(SSDP_MULTICAST_ADDR);

	if(igmp_leavegroup(localIp, multicastIp) != ERR_OK) {
		debug_e("SSDP failed to leave igmp group\n");
	}

	HttpHeaders headers;
	headers["NTS"] = "ssdp:byebye";
	headers["Cache-Control"] = "max-age=1800";

	HttpRequest request(Url("http://" + ip.toString() + ":" + port).Path = "*");
	request.setMethod(HTTP_NOTIFY);
	request.setHeaders(headers);

	sendStringTo(ip, port, request.toString());
	UdpConnection::close();
}

bool Ssdp::search(const String& searchTarget)
{
	HttpHeaders headers;
	if(searchTarget != nullptr) {
		headers["ST"] = searchTarget;
	} else {
		headers["ST"] = "ssdp:all";
	}
	headers["Cache-Control"] = "max-age=1800";
	headers["Man"] = "\"ssdp:discover\"";
	headers["MX"] = "1";

	Url url;
	url.Host = ip.toString();
	url.Port = port;
	url.Path = "*";

	HttpRequest request(url);
	request.setMethod(HTTP_MSEARCH);
	request.setHeaders(headers);

	return sendStringTo(ip, port, request.toString());
}

bool Ssdp::process(char* data, size_t length, IpAddress remoteIp, uint16_t remotePort)
{
	this->remoteIp = remoteIp;
	this->remotePort = remotePort;
	int parsedBytes = http_parser_execute(&parser, &parserSettings, data, length);
	if(HTTP_PARSER_ERRNO(&parser) != HPE_OK) {
		debug_e("Got error: %d from: %s:%d", HTTP_PARSER_ERRNO(&parser), remoteIp.toString().c_str(), remotePort);
		return false;
	}

	return true;
}

void Ssdp::onReceive(pbuf* buf, IpAddress remoteIp, uint16_t remotePort)
{
	pbuf* cur = buf;
	while(cur != nullptr && cur->len > 0) {
		bool success = process((char*)cur->payload, cur->len, remoteIp, remotePort);
		if(!success) {
			debug_e("Ssdp::onReceive: Aborted from receive callback");
			return;
		}

		cur = cur->next;
	}
}

void Ssdp::resetHeaders()
{
	header.reset();
	incomingHeaders.clear();
}

bool Ssdp::sendFoundResponse()
{
	HttpHeaders headers;
	headers[HTTP_HEADER_CACHE_CONTROL] = "max-age=1800";
	headers[HTTP_HEADER_LOCATION] = settings["baseURL"];
	headers["EXT"] = "";
	headers["ST"] = settings["deviceType"];
	headers["USN"] = settings["UDN"];

	HttpResponse response;
	response.headers.setMultiple(headers);

	return sendStringTo(remoteIp, remotePort, response.toString());
}

int Ssdp::onHeadersComplete(const HttpHeaders& headers, http_parser* parser)
{
	if(parser->type == HTTP_REQUEST) {
		// find the device ST that is looked after and if we are one of the desired type - report back
		bool isSearched = (headers["ST"] == settings["deviceType"] || headers["ST"] == "ssdp:all");
		debug_d("ST header: %s, deviceType: %s, found: %d", headers["ST"].c_str(), settings["deviceType"],
				(headers["ST"] == settings["deviceType"]));
		if(isSearched) {
			sendFoundResponse();
		}

		return 0;
	}

	if(onResponse) {
		onResponse(headers);
	}

	/*
	 * In a HTTP_RESPONSE parser returning '1' from on_headers_complete will tell the parser that it
	 * should not expect a body.
	 */
	return 1;
}

int Ssdp::onMessageComplete(http_parser* parser)
{
	debug_d("Finished message");

	// Reset the parser to work with both REQUEST and RESPONSE messages
	parser->type = HTTP_BOTH;
	parser->flags = 0;
	parser->state = 2;

	return 0;
}

int Ssdp::staticOnHeaderField(http_parser* parser, const char* at, size_t length)
{
	GET_CONNECTION()

	return connection->header.onHeaderField(at, length);
}

int Ssdp::staticOnHeaderValue(http_parser* parser, const char* at, size_t length)
{
	GET_CONNECTION()

	return connection->header.onHeaderValue(connection->incomingHeaders, at, length);
}

int Ssdp::staticOnHeadersComplete(http_parser* parser)
{
	GET_CONNECTION()

	debug_d("The headers are complete");

	/* Callbacks should return non-zero to indicate an error. The parser will
	 * then halt execution.
	 *
	 * The one exception is on_headers_complete. In a HTTP_RESPONSE parser
	 * returning '1' from on_headers_complete will tell the parser that it
	 * should not expect a body. This is used when receiving a response to a
	 * HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
	 * chunked' headers that indicate the presence of a body.
	 *
	 * Returning `2` from on_headers_complete will tell parser that it should not
	 * expect neither a body nor any further responses on this connection. This is
	 * useful for handling responses to a CONNECT request which may not contain
	 * `Upgrade` or `Connection: upgrade` headers.
	 */
	int error = connection->onHeadersComplete(connection->incomingHeaders, parser);
	connection->resetHeaders();

	return error;
}

int Ssdp::staticOnMessageComplete(http_parser* parser)
{
	GET_CONNECTION()

	int error = connection->onMessageComplete(parser);
	connection->resetHeaders();

	return error;
}
