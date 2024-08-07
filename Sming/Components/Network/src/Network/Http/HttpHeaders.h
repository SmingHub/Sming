/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpHeaders.h
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

#pragma once

#include "HttpHeaderFields.h"
#include "WHashMap.h"
#include "DateTime.h"

/** @brief Encapsulates a set of HTTP header information
 *  @note fields are stored as a map of field names vs. values.
 *  Standard fields may be accessed using enumeration tags.
 *  Behaviour is as for HashMap, with the addition of methods to support enumerated field names.
 *
 *  @todo add name and/or value escaping
 *  @ingroup http
 */
class HttpHeaders : public HttpHeaderFields, private HashMap<HttpHeaderFieldName, String>
{
public:
	class HeaderConst : public BaseElement<true>
	{
	public:
		HeaderConst(const HttpHeaderFields& fields, const HttpHeaderFieldName& key, const String& value)
			: BaseElement(key, value), fields(fields)
		{
		}

		String getFieldName() const;
		operator String() const;
		size_t printTo(Print& p) const;

	private:
		const HttpHeaderFields& fields;
	};

	class Iterator : public HashMap::Iterator<true>
	{
	public:
		Iterator(const HttpHeaders& headers, unsigned index)
			: HashMap::Iterator<true>::Iterator(headers, index), fields(headers)
		{
		}

		HeaderConst operator*()
		{
			return HeaderConst(fields, map.keyAt(index), map.valueAt(index));
		}

		HeaderConst operator*() const
		{
			return HeaderConst(fields, map.keyAt(index), map.valueAt(index));
		}

	private:
		const HttpHeaderFields& fields;
	};

	HttpHeaders() = default;

	HttpHeaders(const HttpHeaders& headers) : HttpHeaders()
	{
		setMultiple(headers);
	}

	Iterator begin() const
	{
		return Iterator(*this, 0);
	}

	Iterator end() const
	{
		return Iterator(*this, count());
	}

	using HashMap::operator[];

	/** @brief Fetch a reference to the header field value by name
	 *  @param name
	 *  @retval const String& Reference to value
	 *  @note if the field doesn't exist a null String reference is returned
	 */
	const String& operator[](const String& name) const;

	/** @brief Fetch a reference to the header field value by name
	 *  @param name
	 *  @retval String& Reference to value
	 *  @note if the field doesn't exist it is created with the default null value
	 */
	String& operator[](const String& name)
	{
		return operator[](findOrCreate(name));
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

	template <bool is_const> String operator[](const BaseElement<is_const>& elem) const
	{
		return toString(elem.key(), elem.value());
	}

	using HashMap::contains;

	/**
	 * @brief Determine if given header field is present
	 */
	bool contains(const String& name) const
	{
		return contains(fromString(name));
	}

	using HashMap::remove;

	/**
	 * @brief Append value to multi-value field
	 * @param name
	 * @param value
	 * @retval bool false if value exists and field does not permit multiple values
	 */
	bool append(const HttpHeaderFieldName& name, const String& value);

	void remove(const String& name)
	{
		remove(fromString(name));
	}

	void setMultiple(const HttpHeaders& headers);

	HttpHeaders& operator=(const HttpHeaders& headers)
	{
		clear();
		setMultiple(headers);
		return *this;
	}

	void clear()
	{
		HttpHeaderFields::clear();
		HashMap::clear();
	}

	using HashMap::count;

	DateTime getLastModifiedDate() const
	{
		DateTime dt;
		String strLM = operator[](HTTP_HEADER_LAST_MODIFIED);
		return dt.fromHttpDate(strLM) ? dt : DateTime();
	}

	DateTime getServerDate() const
	{
		DateTime dt;
		String strSD = operator[](HTTP_HEADER_DATE);
		return dt.fromHttpDate(strSD) ? dt : DateTime();
	}
};
