/*
 * HttpHeaders.h
 *
 *  Created on: 28 Jul 2018
 *      Author: mikee47
 *
 *  Encapsulate encoding and decoding of HTTP headers
 *  Used for HTTP connections and SMTP mail
 *
 *  The HttpHeaders class was an empty HashMap class living in 'Structures.h'.
 *  It has been expanded here to simplify usage and clean up messy code.
 */

#ifndef _SMINGCORE_NETWORK_HTTPHEADERS_H_
#define _SMINGCORE_NETWORK_HTTPHEADERS_H_

#include "WString.h"
#include "WHashMap.h"

/*
 * Common HTTP header field names. Enumerating these simplifies matching
 * and generating headers. The strings themselves are stored in flash to
 * save RAM.
 *
 * To improve search efficiency the list is ordered by first character.
 * According to RFC 2616: 4.2, field names are case-insensitive.
 *
 */
#define HTTP_HEADER_FIELDNAME_MAP(XX) \
	XX(AccessControlAllowOrigin, 	"Access-Control-Allow-Origin",    "") \
	XX(Authorization,               "Authorization",                  "Basic") \
	XX(Cc,                          "Cc",                             "email") \
	XX(Connection,                  "Connection",                     "") \
	XX(ContentDisposition,          "Content-Disposition",            "") \
	XX(ContentEncoding,            	"Content-Encoding",               "") \
	XX(ContentLength,               "Content-Length",                 "") \
	XX(ContentType,                 "Content-Type",                   "") \
	XX(ContentTransferEncoding,     "Content-Transfer-Encoding",      "") \
	XX(CacheControl,                "Cache-Control",                  "") \
	XX(Date,                        "Date",                           "") \
	XX(ETag,                        "ETag",                           "") \
	XX(From,                        "From",                           "email") \
	XX(Host,						"Host",                           "") \
	XX(IfMatch,						"If-Match",                       "") \
	XX(IfModifiedSince,				"If-Modified-Since",              "") \
	XX(LastModified,             	"Last-Modified",                  "") \
	XX(Location,                    "Location",                       "") \
	XX(SecWebSocketAccept,         	"Sec-WebSocket-Accept",           "") \
	XX(SecWebSocketVersion,         "Sec-WebSocket-Version",          "") \
	XX(SecWebSocketKey,         	"Sec-WebSocket-Key",			  "") \
	XX(SecWebSocketProtocol,        "Sec-WebSocket-Protocol",		  "") \
	XX(Server,                      "Server",                         "") \
	XX(SetCookie,                   "Set-Cookie",                     "") \
	XX(Subject,                     "Subject",                        "email") \
	XX(To,                          "To",                             "email") \
	XX(TransferEncoding,            "Transfer-Encoding",              "Chunked/quoted-printable") \
	XX(Upgrade,                     "Upgrade",                        "") \
	XX(UserAgent,                   "User-Agent",                     "") \
	XX(WWWAuthenticate,             "WWW-Authenticate",               "")

enum HttpHeaderFieldName
{
	hhfn_UNKNOWN = 0,
#define XX(_tag, _str, _comment) \
	hhfn_ ## _tag,
	HTTP_HEADER_FIELDNAME_MAP(XX)
#undef XX
	hhfn_MAX
};


/** @brief Encapsulates a set of HTTP header information
 *  @note fields are stored as a map of field names vs. values.
 *  Standard fields may be accessed using enumeration tags.
 *
 *  @todo add value escaping
 */
class HttpHeaders: public HashMap<String, String>
{
public:
	static String toString(HttpHeaderFieldName name);

	/** @brief Produce a string for output in the HTTP header, with line ending
	 *
	 */
	static String toString(const String& name, const String& value)
	{
		return name + ": " + value + "\r\n";
	}

	static String toString(HttpHeaderFieldName name, const String& value)
	{
		return toString(toString(name), value);
	}

	/** @brief Find the enumerated value for the given field name
	 *  @param name
	 *  @retval HttpHeaderFieldName field name code, hhfn_UNKNOWN if not recognised
	 *  @note comparison is not case-sensitive
	 */
	static HttpHeaderFieldName fromString(const String& name);

	// Override HashMap version so non-existent value doesn't get created
	const String& operator[](const String& name) const
	{
		return getValue(name);
	}
	const String& operator[](HttpHeaderFieldName name) const
	{
		return getValue(toString(name));
	}

	using HashMap::operator[];

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

    HttpHeaders& operator = (const HttpHeaders& headers)
	{
		clear();
		setMultiple(headers);
		return *this;
	}

private:
	// Get a value without creating it, default to empty string
	const String& getValue(const String& name) const
	{
		int i = indexOf(name);
		return (i >= 0) ? operator[](name) : String::nullstr;
	}

};


#endif /* _SMINGCORE_NETWORK_HTTPHEADERS_H_ */
