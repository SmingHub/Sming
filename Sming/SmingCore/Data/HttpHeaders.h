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

#ifndef _SMING_CORE_DATA_HTTP_HEADERS_H_
#define _SMING_CORE_DATA_HTTP_HEADERS_H_

#include "WString.h"
#include "WHashMap.h"

/*
 * Common HTTP header field names. Enumerating these simplifies matching
 * and generating headers. The strings themselves are stored in flash to
 * save RAM.
 *
 * According to RFC 2616: 4.2, field names are case-insensitive.
 *
 */
#define HTTP_HEADER_FIELDNAME_MAP(XX)                                                                                  \
	XX(AccessControlAllowOrigin, "Access-Control-Allow-Origin", "")                                                    \
	XX(Authorization, "Authorization", "Basic")                                                                        \
	XX(Cc, "Cc", "email")                                                                                              \
	XX(Connection, "Connection", "")                                                                                   \
	XX(ContentDisposition, "Content-Disposition", "")                                                                  \
	XX(ContentEncoding, "Content-Encoding", "")                                                                        \
	XX(ContentLength, "Content-Length", "")                                                                            \
	XX(ContentType, "Content-Type", "")                                                                                \
	XX(ContentTransferEncoding, "Content-Transfer-Encoding", "")                                                       \
	XX(CacheControl, "Cache-Control", "")                                                                              \
	XX(Date, "Date", "")                                                                                               \
	XX(ETag, "ETag", "")                                                                                               \
	XX(From, "From", "email")                                                                                          \
	XX(Host, "Host", "")                                                                                               \
	XX(IfMatch, "If-Match", "")                                                                                        \
	XX(IfModifiedSince, "If-Modified-Since", "")                                                                       \
	XX(LastModified, "Last-Modified", "")                                                                              \
	XX(Location, "Location", "")                                                                                       \
	XX(SecWebSocketAccept, "Sec-WebSocket-Accept", "")                                                                 \
	XX(SecWebSocketVersion, "Sec-WebSocket-Version", "")                                                               \
	XX(SecWebSocketKey, "Sec-WebSocket-Key", "")                                                                       \
	XX(SecWebSocketProtocol, "Sec-WebSocket-Protocol", "")                                                             \
	XX(Server, "Server", "")                                                                                           \
	XX(SetCookie, "Set-Cookie", "")                                                                                    \
	XX(Subject, "Subject", "email")                                                                                    \
	XX(To, "To", "email")                                                                                              \
	XX(TransferEncoding, "Transfer-Encoding", "Chunked/quoted-printable")                                              \
	XX(Upgrade, "Upgrade", "")                                                                                         \
	XX(UserAgent, "User-Agent", "")                                                                                    \
	XX(WWWAuthenticate, "WWW-Authenticate", "")

enum HttpHeaderFieldName {
	hhfn_UNKNOWN = 0,
#define XX(_tag, _str, _comment) hhfn_##_tag,
	HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX
		hhfn_MAX
};

/** @brief Encapsulates a set of HTTP header information
 *  @note fields are stored as a map of field names vs. values.
 *  Standard fields may be accessed using enumeration tags.
 *  Behaviour is as for HashMap, with the addition of methods to support enumerated field names.
 *
 *  @todo add name and/or value escaping
 */
class HttpHeaders : public HashMap<String, String>
{
public:
	HttpHeaders();

	static String toString(HttpHeaderFieldName name);

	/** @brief Produce a string for output in the HTTP header, with line ending
	 *  @param name
	 *  @param value
	 *  @retval String
	 */
	static String toString(const String& name, const String& value)
	{
		return name + ": " + value + "\r\n";
	}

	static String toString(HttpHeaderFieldName name, const String& value)
	{
		return toString(toString(name), value);
	}

	/** @brief Find the enumerated value for the given field name string
	 *  @param name
	 *  @retval HttpHeaderFieldName field name code, hhfn_UNKNOWN if not recognised
	 *  @note comparison is not case-sensitive
	 */
	static HttpHeaderFieldName fromString(const String& name);

	using HashMap::operator[];

	const String& operator[](HttpHeaderFieldName name) const
	{
		return operator[](toString(name));
	}

	String& operator[](HttpHeaderFieldName name)
	{
		return operator[](toString(name));
	}

	String operator[](unsigned index) const
	{
		return toString(keyAt(index), valueAt(index));
	}

	using HashMap::contains;

	bool contains(HttpHeaderFieldName name)
	{
		return contains(toString(name));
	}

	using HashMap::remove;

	void remove(HttpHeaderFieldName name)
	{
		remove(toString(name));
	}

	HttpHeaders& operator=(const HttpHeaders& headers)
	{
		clear();
		setMultiple(headers);
		return *this;
	}
};

#endif /* _SMING_CORE_DATA_HTTP_HEADERS_H_ */
