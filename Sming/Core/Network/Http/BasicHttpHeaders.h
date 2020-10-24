/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BasicHttpHeaders.h - in-place HTTP header parsing
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "HttpCommon.h"
#include "HttpHeaderFields.h"

/**
 * @brief Parse array of name/value pairs as references to original data
 * @note When parsing a fixed block of text we don't need to make copies of the content,
 * just nul-terminate the elements and build a list of references.
 */
class BasicHttpHeaders : public HttpHeaderFields
{
public:
	// Uses an array...
	static constexpr size_t maxValues = 16;

	// ...of these
	struct Header {
		const char* name;
		const char* value;

		/**
		 * @brief Return a String in the form "name: value\r\n"
		 */
		String toString() const
		{
			String s;
			s += name;
			s += ": ";
			s += value;
			s += "\r\n";
			return s;
		}

		operator String() const
		{
			return toString();
		}
	};

	BasicHttpHeaders()
	{
		clear();
	}

	/**
	 * @brief Reset to default state
	 */
	void clear();

	/**
	 * @brief Parse header data into name/value pairs
	 * @param data
	 * @param len
	 * @param type Type of headers to parse. The default (HTTP_BOTH) detects this automatically,
	 * use `type()` to determine which. Specifying HTTP_REQUEST or HTTP_RESPONSE will only accept
	 * the given type and fail on mismatch.
	 * @retval http_errno Result of parsing, HPE_OK on success.
	 * Can use with `httpGetErrorName()` or `httpGetErrorDescription()`.
	 * @note Content of provided data is modified to insert NUL terminators on string values
	 * Use type() method to determine whether it's a request or response
	 */
	http_errno parse(char* data, size_t len, http_parser_type type = HTTP_BOTH);

	const Header& operator[](unsigned i) const
	{
		return headers[i];
	}

	String toString(unsigned i) const
	{
		return operator[](i).toString();
	}

	const char*& operator[](const char* name);

	/**
	 * @brief Get number of parsed headers
	 */
	unsigned count() const
	{
		return count_;
	}

	/**
	 * @brief Find a header by name
	 * @param name Case-insensitive
	 * @retval const char* If found, the value, otherwise nullptr
	 */
	const char* operator[](const char* name) const;

	const char* operator[](HttpHeaderFieldName name) const
	{
		return operator[](HttpHeaderFields::toString(name).c_str());
	}

	bool contains(const char* name) const
	{
		return operator[](name) != nullptr;
	}

	bool contains(HttpHeaderFieldName name) const
	{
		return operator[](name) != nullptr;
	}

	/**
	 * @brief Get the type of message parsed
	 * @retval http_parser_type either HTTP_REQUEST or HTTP_RESPONSE
	 */
	http_parser_type type() const
	{
		return http_parser_type(parser.type);
	}

	/**
	 * @brief Obtain request method
	 */
	http_method method() const
	{
		return http_method(parser.method);
	}

	void setMethod(http_method method)
	{
		parser.method = unsigned(method);
	}

	/**
	 * @brief Obtain text for request method
	 */
	const char* methodStr() const
	{
		return http_method_str(method());
	}

	/**
	 * @brief Obtain response status
	 */
	http_status status() const
	{
		return http_status(parser.status_code);
	}

	String statusText() const
	{
		return httpGetStatusText(status());
	}

	/**
	 * @brief Obtain content length field value
	 */
	unsigned contentLength() const
	{
		return parser.content_length;
	}

private:
	static int staticOnField(http_parser* parser, const char* at, size_t length);
	static int staticOnValue(http_parser* parser, const char* at, size_t length);
	int onField(const char* at, size_t length);
	int onValue(const char* at, size_t length);

private:
	http_parser parser;
	static const http_parser_settings parserSettings;
	Header headers[maxValues];
	unsigned count_ = 0;
	const char* nullValue = nullptr;
};
