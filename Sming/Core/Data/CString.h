/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CString.h
 *
 * @author: 2020 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <WString.h>
#include <memory>

/**
 * @brief Class to manage a NUL-terminated C-style string
 * When storing persistent strings in RAM the regular String class can become inefficient,
 * so using a regular `char*` can be preferable. This class provides that with additional
 * methods to simplify lifetime management and provide some interoperability with
 * Wiring String objects.
 */
class CString : public std::unique_ptr<char[]>
{
public:
	CString() = default;

	CString(const CString& src) = default;

	CString(const String& src)
	{
		assign(src);
	}

	void assign(const String& src)
	{
		assign(src.c_str(), src.length());
	}

	void assign(const char* src)
	{
		assign(src, src ? strlen(src) : 0);
	}

	void assign(const char* src, size_t len)
	{
		if(src == nullptr || len == 0) {
			reset();
		} else {
			++len;
			reset(new char[len]);
			memcpy(get(), src, len);
		}
	}

	CString& operator=(const String& src)
	{
		assign(src);
		return *this;
	}

	CString& operator=(const char* src)
	{
		assign(src);
		return *this;
	}

	const char* c_str() const
	{
		return get() ?: "";
	}

	bool operator==(const CString& other) const
	{
		return strcmp(c_str(), other.c_str()) == 0;
	}

	bool operator==(const String& other) const
	{
		return strcmp(c_str(), other.c_str()) == 0;
	}

	size_t length() const
	{
		auto p = get();
		return p ? strlen(p) : 0;
	}

	explicit operator String() const
	{
		return get();
	}
};
