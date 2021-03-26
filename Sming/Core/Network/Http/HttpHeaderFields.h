/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpHeaderFields.h - Standard HTTP header field definitions
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "Data/CStringArray.h"
#include "WString.h"

/*
 * Common HTTP header field names. Enumerating these simplifies matching
 * and generating headers. The strings themselves are stored in flash to
 * save RAM.
 *
 * According to RFC 2616: 4.2, field names are case-insensitive.
 *
 * A brief description of each header field is given for information purposes.
 * For details see https://www.iana.org/assignments/message-headers/message-headers.xhtml
 *
 */
#define HTTP_HEADER_FIELDNAME_MAP(XX)                                                                                  \
	XX(ACCEPT, "Accept", "Limit acceptable response types")                                                            \
	XX(ACCESS_CONTROL_ALLOW_ORIGIN, "Access-Control-Allow-Origin", "")                                                 \
	XX(AUTHORIZATION, "Authorization", "Basic user agent authentication")                                              \
	XX(CC, "Cc", "email field")                                                                                        \
	XX(CONNECTION, "Connection", "Indicates sender's desired control options")                                         \
	XX(CONTENT_DISPOSITION, "Content-Disposition", "Additional information about how to process response payload")     \
	XX(CONTENT_ENCODING, "Content-Encoding", "Applied encodings in addition to content type")                          \
	XX(CONTENT_LENGTH, "Content-Length", "Anticipated size for payload when not using transfer encoding")              \
	XX(CONTENT_TYPE, "Content-Type",                                                                                   \
	   "Payload media type indicating both data format and intended manner of processing by recipient")                \
	XX(CONTENT_TRANSFER_ENCODING, "Content-Transfer-Encoding", "Coding method used in a MIME message body part")       \
	XX(CACHE_CONTROL, "Cache-Control", "Directives for caches along the request/response chain")                       \
	XX(DATE, "Date", "Message originating date/time")                                                                  \
	XX(EXPECT, "Expect", "Behaviours to be supported by the server in order to properly handle this request.")         \
	XX(ETAG, "ETag",                                                                                                   \
	   "Validates resource, such as a file, so recipient can confirm whether it has changed - generally more "         \
	   "reliable than Date")                                                                                           \
	XX(FROM, "From", "email address of human user who controls the requesting user agent")                             \
	XX(HOST, "Host",                                                                                                   \
	   "Request host and port information for target URI; allows server to service requests for multiple hosts on a "  \
	   "single IP address")                                                                                            \
	XX(IF_MATCH, "If-Match",                                                                                           \
	   "Precondition check using ETag to avoid accidental overwrites when servicing multiple user requests. Ensures "  \
	   "resource entity tag matches before proceeding.")                                                               \
	XX(IF_MODIFIED_SINCE, "If-Modified-Since", "Precondition check using Date")                                        \
	XX(LAST_MODIFIED, "Last-Modified", "Server timestamp indicating date and time resource was last modified")         \
	XX(LOCATION, "Location", "Used in redirect responses, amongst other places")                                       \
	XX(SEC_WEBSOCKET_ACCEPT, "Sec-WebSocket-Accept", "Server response to opening Websocket handshake")                 \
	XX(SEC_WEBSOCKET_VERSION, "Sec-WebSocket-Version",                                                                 \
	   "Websocket opening request indicates acceptable protocol version. Can appear more than once.")                  \
	XX(SEC_WEBSOCKET_KEY, "Sec-WebSocket-Key", "Websocket opening request validation key")                             \
	XX(SEC_WEBSOCKET_PROTOCOL, "Sec-WebSocket-Protocol",                                                               \
	   "Websocket opening request indicates supported protocol(s), response contains negotiated protocol(s)")          \
	XX(SERVER, "Server", "Identifies software handling requests")                                                      \
	XX(SET_COOKIE, "Set-Cookie", "Server may pass name/value pairs and associated metadata to user agent (client)")    \
	XX(SUBJECT, "Subject", "email subject line")                                                                       \
	XX(TO, "To", "email intended recipient address")                                                                   \
	XX(TRANSFER_ENCODING, "Transfer-Encoding", "e.g. Chunked, compress, deflate, gzip")                                \
	XX(UPGRADE, "Upgrade",                                                                                             \
	   "Used to transition from HTTP to some other protocol on the same connection. e.g. Websocket")                   \
	XX(USER_AGENT, "User-Agent", "Information about the user agent originating the request")                           \
	XX(WWW_AUTHENTICATE, "WWW-Authenticate", "Indicates HTTP authentication scheme(s) and applicable parameters")      \
	XX(PROXY_AUTHENTICATE, "Proxy-Authenticate", "Indicates proxy authentication scheme(s) and applicable parameters")

enum class HttpHeaderFieldName {
	UNKNOWN = 0,
#define XX(tag, str, comment) tag,
	HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX
		CUSTOM // First custom header tag value
};

#define XX(tag, str, comment) constexpr HttpHeaderFieldName HTTP_HEADER_##tag = HttpHeaderFieldName::tag;
XX(UNKNOWN, "", "")
HTTP_HEADER_FIELDNAME_MAP(XX)
XX(CUSTOM, "", "")
#undef XX

class HttpHeaderFields
{
public:
	/**
	 * @brief Checks if a header is allowed to have multiple values
	 * @retval bool true if allowed
	 */
	bool isMultiHeader(HttpHeaderFieldName name) const
	{
		switch(name) {
		case HTTP_HEADER_SET_COOKIE:
		case HTTP_HEADER_WWW_AUTHENTICATE:
		case HTTP_HEADER_PROXY_AUTHENTICATE:
			break;
		default:
			return false;
		}

		return true;
	}

	String toString(HttpHeaderFieldName name) const;

	/** @brief Produce a string for output in the HTTP header, with line ending
	 *  @param name
	 *  @param value
	 *  @retval String
	 */
	static String toString(const String& name, const String& value);

	String toString(HttpHeaderFieldName name, const String& value) const;

	/** @brief Find the enumerated value for the given field name string
	 *  @param name
	 *  @retval HttpHeaderFieldName field name code, HTTP_HEADER_UNKNOWN if not recognised
	 *  @note comparison is not case-sensitive
	 */
	HttpHeaderFieldName fromString(const String& name) const;

	/** @brief Find the enumerated value for the given field name string, create a custom entry if not found
	 *  @param name
	 *  @retval HttpHeaderFieldName field name code
	 *  @note comparison is not case-sensitive
	 */
	HttpHeaderFieldName findOrCreate(const String& name)
	{
		auto field = fromString(name);
		if(field == HTTP_HEADER_UNKNOWN) {
			field = static_cast<HttpHeaderFieldName>(unsigned(HTTP_HEADER_CUSTOM) + customFieldNames.count());
			customFieldNames.add(name);
		}
		return field;
	}

	void clear()
	{
		customFieldNames.clear();
	}

private:
	/** @brief Try to match a string against the list of custom field names
	 *  @param name
	 *  @retval HttpHeaderFieldName HTTP_HEADER_UNKNOWN if not found
	 */
	HttpHeaderFieldName findCustomFieldName(const String& name) const;

	CStringArray customFieldNames;
};
