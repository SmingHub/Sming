/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 *  Encapsulate encoding and decoding of HTTP header fields
 *  Used for HTTP connections and SMTP mail
 *
 *  The HttpHeaders class was an empty HashMap class living in 'Structures.h'.
 *  It has been expanded here to simplify code, and to provide enumerated keys
 *  for common field names.
 *
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_HEADERS_H_
#define _SMING_CORE_NETWORK_HTTP_HEADERS_H_

#include "Data/CStringArray.h"
#include "WString.h"
#include "WHashMap.h"

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
	XX(WWW_AUTHENTICATE, "WWW-Authenticate", "Indicates HTTP authentication scheme(s) and applicable parameters")

enum HttpHeaderFieldName {
	HTTP_HEADER_UNKNOWN = 0,
#define XX(_tag, _str, _comment) HTTP_HEADER_##_tag,
	HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX
		HTTP_HEADER_CUSTOM // First custom header tag value
};

/** @brief Encapsulates a set of HTTP header information
 *  @note fields are stored as a map of field names vs. values.
 *  Standard fields may be accessed using enumeration tags.
 *  Behaviour is as for HashMap, with the addition of methods to support enumerated field names.
 *
 *  @todo add name and/or value escaping
 */
class HttpHeaders : protected HashMap<HttpHeaderFieldName, String>
{
public:
	HttpHeaders()
	{
	}

	HttpHeaders(const HttpHeaders& headers)
	{
		*this = headers;
	}

	String toString(HttpHeaderFieldName name) const;

	/** @brief Produce a string for output in the HTTP header, with line ending
	 *  @param name
	 *  @param value
	 *  @retval String
	 */
	static String toString(const String& name, const String& value);

	String toString(HttpHeaderFieldName name, const String& value) const
	{
		return toString(toString(name), value);
	}

	/** @brief Find the enumerated value for the given field name string
	 *  @param name
	 *  @retval HttpHeaderFieldName field name code, HTTP_HEADER_UNKNOWN if not recognised
	 *  @note comparison is not case-sensitive
	 */
	HttpHeaderFieldName fromString(const String& name) const;

	using HashMap::operator[];

	/** @brief Fetch a reference to the header field value by name
	 *  @param name
	 *  @retval const String& Reference to value
	 *  @note if the field doesn't exist a null String reference is returned
	 */
	const String& operator[](const String& name) const
	{
		auto field = fromString(name);
		if(field == HTTP_HEADER_UNKNOWN) {
			return nil;
		}
		return operator[](field);
	}

	/** @brief Fetch a reference to the header field value by name
	 *  @param name
	 *  @retval String& Reference to value
	 *  @note if the field doesn't exist it is created with the default null value
	 */
	String& operator[](const String& name)
	{
		auto field = fromString(name);
		if(field == HTTP_HEADER_UNKNOWN) {
			field = static_cast<HttpHeaderFieldName>(HTTP_HEADER_CUSTOM + customFieldNames.count());
			customFieldNames.add(name);
		}
		return operator[](field);
	}

	/** @brief Return the HTTP header line for the value at the given index
	 *  @param index
	 *  @retval String
	 *  @note if the index is invalid,
	 */
	String operator[](unsigned index) const
	{
		return toString(keyAt(index), valueAt(index));
	}

	using HashMap::contains;

	bool contains(const String& name)
	{
		return contains(fromString(name));
	}

	using HashMap::remove;

	void remove(const String& name)
	{
		remove(fromString(name));
	}

	void setMultiple(const HttpHeaders& headers)
	{
		for(unsigned i = 0; i < headers.count(); i++) {
			HttpHeaderFieldName fieldName = headers.keyAt(i);
			auto fieldNameString = headers.toString(fieldName);
			operator[](fieldNameString) = headers.valueAt(i);
		}
	}

	HttpHeaders& operator=(const HttpHeaders& headers)
	{
		clear();
		setMultiple(headers);
		return *this;
	}

	void clear()
	{
		customFieldNames.clear();
		HashMap::clear();
	}

	using HashMap::count;

private:
	/** @brief Try to match a string against the list of custom field names
	 *  @param name
	 *  @retval HttpHeaderFieldName HTTP_HEADER_UNKNOWN if not found
	 */
	HttpHeaderFieldName findCustomFieldName(const String& name) const;

	CStringArray customFieldNames;
};

#endif /* _SMING_CORE_NETWORK_HTTP_HEADERS_H_ */
