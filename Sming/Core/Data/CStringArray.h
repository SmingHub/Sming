/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CStringArray.h
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "WString.h"
#include "stringutil.h"

/**
 * @brief Class to manage a double null-terminated list of strings, such as "one\0two\0three\0"
 */
class CStringArray : private String
{
public:
	// Inherit any safe/useful methods
	using String::compareTo;
	using String::equals;
	using String::reserve;
	using String::operator StringIfHelperType;
	using String::operator==;
	using String::operator!=;
	using String::operator<;
	using String::operator>;
	using String::operator<=;
	using String::operator>=;
	using String::c_str;
	using String::endsWith;
	using String::equalsIgnoreCase;
	using String::getBytes;
	using String::length;
	using String::startsWith;
	using String::toCharArray;
	using String::toLowerCase;
	using String::toUpperCase;

	/**
	 * @name Constructors
	 * @{
	 */
	CStringArray(const String& str) : String(str)
	{
		init();
	}

	CStringArray(const char* cstr = nullptr) : String(cstr)
	{
		init();
	}

	CStringArray(const char* cstr, unsigned int length) : String(cstr, length)
	{
		init();
	}

	explicit CStringArray(flash_string_t pstr, int length = -1) : String(pstr, length)
	{
		init();
	}

	CStringArray(const FlashString& fstr) : String(fstr)
	{
		init();
	}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
	CStringArray(String&& rval) : String(std::move(rval))
	{
		init();
	}
	CStringArray(StringSumHelper&& rval) : String(std::move(rval))
	{
		init();
	}
#endif
	/** @} */

	CStringArray& operator=(const char* cstr)
	{
		String::operator=(cstr);
		init();
		return *this;
	}

	/** @brief Append a new string (or array of strings) to the end of the array
	 *  @param str
	 *  @param length Length of new string in array (default is length of str)
	 *  @retval bool false on memory allocation error
	 *  @note If str contains any NUL characters it will be handled as an array
	 */
	bool add(const char* str, int length = -1);

	/** @brief Append a new string (or array of strings) to the end of the array
	 *  @param str
	 *  @retval bool false on memory allocation error
	 *  @note If str contains any NUL characters it will be handled as an array
	 */
	bool add(const String& str)
	{
		return add(str.c_str(), str.length());
	}

	/**
	 * @name Concatenation operators
	 * @{
	 */
	CStringArray& operator+=(const String& str)
	{
		add(str);
		return *this;
	}

	CStringArray& operator+=(const char* cstr)
	{
		add(cstr);
		return *this;
	}

	/**
	 * @brief Append numbers, etc. to the array
	 * @param value char, int, float, etc. as supported by String
	 */
	template <typename T> CStringArray& operator+=(T value)
	{
		add(String(value));
		return *this;
	}
	/** @} */

	/** @brief Find the given string and return its index
	 * 	@param str String to find
	 * 	@param ignoreCase Whether search is case-sensitive or not
	 * 	@retval int index of given string, -1 if not found
	 * 	@note Comparison is not case-sensitive
	 */
	int indexOf(const char* str, bool ignoreCase = true) const;

	/** @brief Find the given string and return its index
	 * 	@param str String to find
	 * 	@param ignoreCase Whether search is case-sensitive or not
	 * 	@retval int index of given string, -1 if not found
	 * 	@note Comparison is not case-sensitive
	 */
	int indexOf(const String& str, bool ignoreCase = true) const
	{
		return indexOf(str.c_str(), ignoreCase);
	}

	/** @brief  Check if array contains a string
	 *  @param  str String to search for
	 * 	@param ignoreCase Whether search is case-sensitive or not
	 *  @retval bool True if string exists in array
	 *  @note   Search is not case-sensitive
	 */
	bool contains(const char* str, bool ignoreCase = true) const
	{
		return indexOf(str, ignoreCase) >= 0;
	}

	/** @brief  Check if array contains a string
	 *  @param  str String to search for
	 * 	@param ignoreCase Whether search is case-sensitive or not
	 *  @retval bool True if string exists in array
	 *  @note   Search is not case-sensitive
	 */
	bool contains(const String& str, bool ignoreCase = true) const
	{
		return indexOf(str, ignoreCase) >= 0;
	}

	/** @brief Get string at the given position
	 *  @param index 0-based index of string to obtain
	 *  @retval const char* nullptr if index is not valid
	 */
	const char* getValue(unsigned index) const;

	/** @brief Get string at the given position
	 *  @param index 0-based index of string to obtain
	 *  @retval const char* nullptr if index is not valid
	 */
	const char* operator[](unsigned index) const
	{
		return getValue(index);
	}

	/** @brief Empty the array
	 */
	void clear()
	{
		*this = nullptr;
		stringCount = 0;
	}

	/** @brief  Get quantity of strings in array
	*   @retval unsigned Quantity of strings
	*/
	unsigned count() const;

	/**
	 * @name Iterator support (forward only)
	 * @{
	 */
	class Iterator
	{
	public:
		Iterator() = default;
		Iterator(const Iterator&) = default;

		Iterator(const CStringArray* array, uint16_t offset, uint16_t index)
			: array_(array), offset_(offset), index_(index)
		{
		}

		operator bool() const
		{
			return array_ != nullptr && offset_ < array_->length();
		}

		bool equals(const char* rhs) const
		{
			auto s = str();
			return s == rhs || strcmp(s, rhs) == 0;
		}

		bool equalsIgnoreCase(const char* rhs) const
		{
			auto s = str();
			return s == rhs || strcasecmp(str(), rhs) == 0;
		}

		bool operator==(const Iterator& rhs) const
		{
			return array_ == rhs.array_ && offset_ == rhs.offset_;
		}

		bool operator!=(const Iterator& rhs) const
		{
			return !operator==(rhs);
		}

		bool operator==(const char* rhs) const
		{
			return equals(rhs);
		}

		bool operator!=(const char* rhs) const
		{
			return !equals(rhs);
		}

		bool equals(const String& rhs) const
		{
			return rhs.equals(str());
		}

		bool equalsIgnoreCase(const String& rhs) const
		{
			return rhs.equalsIgnoreCase(str());
		}

		bool operator==(const String& rhs) const
		{
			return equals(rhs);
		}

		bool operator!=(const String& rhs) const
		{
			return !equals(rhs);
		}

		const char* str() const
		{
			if(array_ == nullptr) {
				return "";
			}

			return array_->c_str() + offset_;
		}

		const char* operator*() const
		{
			return str();
		}

		uint16_t index() const
		{
			return index_;
		}

		uint16_t offset() const
		{
			return offset_;
		}

		Iterator& operator++()
		{
			next();
			return *this;
		}

		Iterator operator++(int)
		{
			Iterator tmp(*this);
			next();
			return tmp;
		}

		void next()
		{
			if(*this) {
				offset_ += strlen(str()) + 1;
				++index_;
			}
		}

		using const_iterator = Iterator;

	private:
		const CStringArray* array_ = nullptr;
		uint16_t offset_ = 0;
		uint16_t index_ = 0;
	};

	Iterator begin() const
	{
		return Iterator(this, 0, 0);
	}

	Iterator end() const
	{
		return Iterator(this, length(), count());
	}

	/** @} */

private:
	void init();

private:
	mutable unsigned stringCount = 0;
};
