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
#include <Data/BitSet.h>

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
 * Entries are formatted thus: XX(tag, str, flags, comment)
 *   tag:     Identifier used in code (without HTTP_HEADER_ prefix)
 *   str:     String used in HTTP protocol
 *   flags:   Additional flags (see HttpHeaderFields::Flag)
 *   comment: Further details
 */
#define HTTP_HEADER_FIELDNAME_MAP(XX)                                                                                  \
	XX(ACCEPT, "Accept", 0, "Limit acceptable response types")                                                         \
	XX(ACCEPT_ENCODING, "Accept-Encoding", 0, "Limit acceptable content encoding types")                               \
	XX(ACCESS_CONTROL_ALLOW_ORIGIN, "Access-Control-Allow-Origin", 0, "")                                              \
	XX(AUTHORIZATION, "Authorization", 0, "Basic user agent authentication")                                           \
	XX(CC, "Cc", 0, "email field")                                                                                     \
	XX(CONNECTION, "Connection", 0, "Indicates sender's desired control options")                                      \
	XX(CONTENT_DISPOSITION, "Content-Disposition", 0, "Additional information about how to process response payload")  \
	XX(CONTENT_ENCODING, "Content-Encoding", 0, "Applied encodings in addition to content type")                       \
	XX(CONTENT_LENGTH, "Content-Length", 0, "Anticipated size for payload when not using transfer encoding")           \
	XX(CONTENT_TYPE, "Content-Type", 0,                                                                                \
	   "Payload media type indicating both data format and intended manner of processing by recipient")                \
	XX(CONTENT_TRANSFER_ENCODING, "Content-Transfer-Encoding", 0, "Coding method used in a MIME message body part")    \
	XX(CACHE_CONTROL, "Cache-Control", 0, "Directives for caches along the request/response chain")                    \
	XX(DATE, "Date", 0, "Message originating date/time")                                                               \
	XX(EXPECT, "Expect", 0, "Behaviours to be supported by the server in order to properly handle this request.")      \
	XX(ETAG, "ETag", 0,                                                                                                \
	   "Validates resource, such as a file, so recipient can confirm whether it has changed - generally more "         \
	   "reliable than Date")                                                                                           \
	XX(FROM, "From", 0, "email address of human user who controls the requesting user agent")                          \
	XX(HOST, "Host", 0,                                                                                                \
	   "Request host and port information for target URI; allows server to service requests for multiple hosts on a "  \
	   "single IP address")                                                                                            \
	XX(IF_MATCH, "If-Match", 0,                                                                                        \
	   "Precondition check using ETag to avoid accidental overwrites when servicing multiple user requests. Ensures "  \
	   "resource entity tag matches before proceeding.")                                                               \
	XX(IF_MODIFIED_SINCE, "If-Modified-Since", 0, "Precondition check using Date")                                     \
	XX(LAST_MODIFIED, "Last-Modified", 0, "Server timestamp indicating date and time resource was last modified")      \
	XX(LOCATION, "Location", 0, "Used in redirect responses, amongst other places")                                    \
	XX(SEC_WEBSOCKET_ACCEPT, "Sec-WebSocket-Accept", 0, "Server response to opening Websocket handshake")              \
	XX(SEC_WEBSOCKET_VERSION, "Sec-WebSocket-Version", 0,                                                              \
	   "Websocket opening request indicates acceptable protocol version. Can appear more than once.")                  \
	XX(SEC_WEBSOCKET_KEY, "Sec-WebSocket-Key", 0, "Websocket opening request validation key")                          \
	XX(SEC_WEBSOCKET_PROTOCOL, "Sec-WebSocket-Protocol", 0,                                                            \
	   "Websocket opening request indicates supported protocol(s), response contains negotiated protocol(s)")          \
	XX(SERVER, "Server", 0, "Identifies software handling requests")                                                   \
	XX(SET_COOKIE, "Set-Cookie", Flag::Multi,                                                                          \
	   "Server may pass name/value pairs and associated metadata to user agent (client)")                              \
	XX(SUBJECT, "Subject", 0, "email subject line")                                                                    \
	XX(TO, "To", 0, "email intended recipient address")                                                                \
	XX(TRANSFER_ENCODING, "Transfer-Encoding", 0, "e.g. Chunked, compress, deflate, gzip")                             \
	XX(UPGRADE, "Upgrade", 0,                                                                                          \
	   "Used to transition from HTTP to some other protocol on the same connection. e.g. Websocket")                   \
	XX(USER_AGENT, "User-Agent", 0, "Information about the user agent originating the request")                        \
	XX(WWW_AUTHENTICATE, "WWW-Authenticate", Flag::Multi,                                                              \
	   "Indicates HTTP authentication scheme(s) and applicable parameters")                                            \
	XX(PROXY_AUTHENTICATE, "Proxy-Authenticate", Flag::Multi,                                                          \
	   "Indicates proxy authentication scheme(s) and applicable parameters")

enum class HttpHeaderFieldName {
	UNKNOWN = 0,
#define XX(tag, str, flags, comment) tag,
	HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX
		CUSTOM // First custom header tag value
};

#define XX(tag, str, flags, comment) constexpr HttpHeaderFieldName HTTP_HEADER_##tag = HttpHeaderFieldName::tag;
XX(UNKNOWN, "", 0, "")
HTTP_HEADER_FIELDNAME_MAP(XX)
XX(CUSTOM, "", 0, "")
#undef XX

class HttpHeaderFields
{
public:
	/**
	 * @brief Flag values providing additional information about header fields
	 */
	enum class Flag {
		Multi, ///< Field may have multiple values
	};
	using Flags = BitSet<uint8_t, Flag, 1>;

	/**
	 * @brief Get flags (if any) for given header field
	 * @retval Flags
	 */
	Flags getFlags(HttpHeaderFieldName name) const;

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
