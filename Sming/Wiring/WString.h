/* $Id: WString.h 1156 2011-06-07 04:01:16Z bhagman $
||
|| @author         Paul Stoffregen <paul@pjrc.com>
|| @url            http://wiring.org.co/
|| @contribution   Hernando Barragan <b@wiring.org.co>
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
||
|| @description
|| | String class.
|| |
|| | Wiring Common API
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

/*
 * @author: 2 Oct 2018 - mikee47 <mike@sillyhouse.net>
 *
 * The standard String object default constructor creates an empty string, which requires a heap allocation of 1 byte.
 * I changed this behaviour to default to a null string (invalid) to avoid this (usually) un-necessary allocation.
 * If the value of the string hasn't actually been assigned yet then an 'invalid' (or null) string is the more logical choice.
 * Additional changes ensure that the content of such a string are equivalent to an empty string "".
 *
 * Background
 *
 * The intent of the Wiring authors seems to be that an expression producing a String object will fail and produce
 * an 'invalid' String (that evaluates to False) if any of the allocations within that expression fail. This could
 * be due to heap fragmentation, low memory or a String which is just too big.
 *
 * By example:
 *
 * 	String tmp = String("A") + String("B");
 *
 * If a heap allocation fails on either "A" or "B" the the result should be a null string. However, this is not actually
 * the case. In practice, if "A" fails but "B" does not then the result will be "B", while if "A" succeeds but "B" fails
 * then the result will be 'invalid'. This would appear to be an oversight in the Wiring library (last updated July 2016).
 *
 * I made a decision with these changes that heap allocation errors are a rare enough occurrence that attempting to deal with
 * them in such a manner causes more problems than it solves.
 *
 * These changes have a knock-on effect in that if any of the allocations in an expression fail, then the result, tmp,
 * will be unpredictable.
 *
 * @author Nov 2019 mikee47 <mike@sillyhouse.net>
 *
 * Small String Optimisation (SSO). Based on the Arduino ESP8266 core implentation.
 * An empty String object now consumes 12 bytes (from 8) but provides an SSO capacity of 11 characters.
 * Capacity and length types changed to size_t, thus String is no longer restricted to 64K.
 *
 */

#pragma once

#ifdef __cplusplus

#include "WConstants.h"
#include <stddef.h>
#include <sming_attr.h>

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#define __GXX_EXPERIMENTAL_CXX0X__
#endif

// When compiling programs with this class, the following gcc parameters
// dramatically increase performance and memory (RAM) efficiency, typically
// with little or no increase in code size.
//     -felide-constructors
//     -std=c++0x

// An inherited class for holding the result of a concatenation.  These
// result objects are assumed to be writable by subsequent concatenations.
class StringSumHelper;

/* Arduino-style flash strings
 *
 * __FlashStringHelper* provides strongly-typed pointer to allow safe implicit
 * operation using String class methods.
 */
class __FlashStringHelper; // Never actually defined
typedef const __FlashStringHelper* flash_string_t;

// Cast a PGM_P (flash memory) pointer to a flash string pointer
#define FPSTR(pstr_pointer) reinterpret_cast<flash_string_t>(pstr_pointer)

/*
 * Use this macro to wrap a string literal and access it using a String object.
 * The string data is stored in flash and only read into RAM when executed.
 * For example: Serial.print(F("This is a test string\n"));
 */
#define F(string_literal) String(FPSTR(PSTR(string_literal)), sizeof(string_literal) - 1)

// Forward declaration for counted flash string - see FlashString.h
struct FlashString;

/**
 * @brief The string class
 *
 * Note that a string object's default constructor creates an empty string.
 * This is not the same as a null string.
 * A null string evaluates to false
 * An empty string evaluates to true
 */
// The string class
class String
{
    // use a function pointer to allow for "if (s)" without the
    // complications of an operator bool(). for more information, see:
    // http://www.artima.com/cppsource/safebool.html
    typedef void (String::*StringIfHelperType)() const;
    void StringIfHelper() const {}

  public:
    // Use these for const references, e.g. in function return values
    static const String nullstr; ///< A null string evaluates to false
    static const String empty; ///< An empty string evaluates to true

    /* constructors

       creates a copy of the initial value.
       if the initial value is null or invalid, or if memory allocation
       fails, the string will be marked as invalid (i.e. "if (s)" will be false).
    */
    String(const char *cstr = nullptr);
    String(const char *cstr, size_t length)
    {
      if (cstr) copy(cstr, length);
    }
    String(const String &str)
    {
      *this = str;
    }
    explicit String(flash_string_t pstr, int length = -1)
    {
      setString(pstr, length);
    }
    String(const FlashString& fstr);

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    String(String && rval)
    {
   	  move(rval);
    }
    String(StringSumHelper && rval);
#endif
    explicit String(char c);
    explicit String(unsigned char, unsigned char base = 10);
    explicit String(int, unsigned char base = 10);
    explicit String(unsigned int, unsigned char base = 10);
    explicit String(long, unsigned char base = 10);
    explicit String(long long, unsigned char base = 10);
    explicit String(unsigned long, unsigned char base = 10);
    explicit String(unsigned long long, unsigned char base = 10);
    explicit String(float, unsigned char decimalPlaces=2);
    explicit String(double, unsigned char decimalPlaces=2);
    ~String(void)
    {
    	invalidate();
    }

    void setString(const char *cstr, int length = -1);
    void setString(flash_string_t pstr, int length = -1);

    // memory management
    // return true on success, false on failure (in which case, the string
    // is left unchanged).  reserve(0), if successful, will validate an
    // invalid string (i.e., "if (s)" will be true afterwards)
    bool reserve(size_t size);

    /** @brief set the string length accordingly, expanding if necessary
     *  @param length required for string (nul terminator additional)
     *  @retval true on success, false on failure
     *  @note extra characters are undefined
     */
    bool setLength(size_t length);

    inline size_t length(void) const
    {
      return sso.set ? sso.len : ptr.len;
    }

    // creates a copy of the assigned value.  if the value is null or
    // invalid, or if the memory allocation fails, the string will be
    // marked as invalid ("if (s)" will be false).
    String & operator = (const String &rhs);
    String & operator = (const char *cstr);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    String & operator = (String && rval)
    {
      if (this != &rval) move(rval);
      return *this;
    }
    String & operator = (StringSumHelper && rval);
#endif

    // concatenate (works w/ built-in types)

    // returns true on success, false on failure (in which case, the string
    // is left unchanged).  if the argument is null or invalid, the
    // concatenation is considered unsucessful.
    bool concat(const String &str)
    {
      return concat(str.cbuffer(), str.length());
    }
    bool concat(const char *cstr);
    bool concat(const char *cstr, size_t length);
    bool concat(char c)
    {
      return concat(&c, 1);
    }
    bool concat(unsigned char num);
    bool concat(int num);
    bool concat(unsigned int num);
    bool concat(long num);
    bool concat(long long num);
    bool concat(unsigned long num);
    bool concat(unsigned long long num);
    bool concat(float num);
    bool concat(double num);
  
    // if there's not enough memory for the concatenated value, the string
    // will be left unchanged (but this isn't signalled in any way)
    String & operator += (const String &rhs)
    {
      concat(rhs);
      return (*this);
    }
    String & operator += (const char *cstr)
    {
      concat(cstr);
      return (*this);
    }
    String & operator += (char c)
    {
      concat(c);
      return (*this);
    }
    String & operator += (unsigned char num)
    {
      concat(num);
      return (*this);
    }
    String & operator += (int num)
    {
      concat(num);
      return (*this);
    }
    String & operator += (unsigned int num)
    {
      concat(num);
      return (*this);
    }
    String & operator += (long num)
    {
      concat(num);
      return (*this);
    }
    String & operator += (long long num)
    {
      concat(num);
      return (*this);
    }
    String & operator += (unsigned long num)
    {
      concat(num);
      return (*this);
    }
    String & operator += (unsigned long long num)
    {
      concat(num);
      return (*this);
    }
    String & operator += (float num)
    {
      concat(num);
      return (*this);
    }
    String & operator += (double num)
    {
      concat(num);
      return (*this);
    }

    friend StringSumHelper & operator + (const StringSumHelper &lhs, const String &rhs);
    friend StringSumHelper & operator + (const StringSumHelper &lhs, const char *cstr);
    friend StringSumHelper & operator + (const StringSumHelper &lhs, char c);
    friend StringSumHelper & operator + (const StringSumHelper &lhs, unsigned char num);
    friend StringSumHelper & operator + (const StringSumHelper &lhs, int num);
    friend StringSumHelper & operator + (const StringSumHelper &lhs, unsigned int num);
    friend StringSumHelper & operator + (const StringSumHelper &lhs, long num);
    friend StringSumHelper & operator + (const StringSumHelper &lhs, unsigned long num);
    friend StringSumHelper & operator + (const StringSumHelper &lhs, unsigned long long num);
    friend StringSumHelper & operator + (const StringSumHelper &lhs, float num);
    friend StringSumHelper & operator + (const StringSumHelper &lhs, double num);

    // comparison (only works w/ Strings and "strings")
    operator StringIfHelperType() const
    {
      return isNull() ? 0 : &String::StringIfHelper;
    }
    int compareTo(const char* cstr, size_t length) const;
    int compareTo(const String &s) const
    {
   	  return compareTo(s.cbuffer(), s.length());
    }
    bool equals(const String &s) const
    {
    	return equals(s.cbuffer(), s.length());
    }
    bool equals(const char *cstr) const;
    bool equals(const char *cstr, size_t length) const;
    bool equals(const FlashString& fstr) const;

    bool operator == (const String &rhs) const
    {
      return equals(rhs);
    }
    bool operator == (const char *cstr) const
    {
      return equals(cstr);
    }
    bool operator==(const FlashString& fstr) const
    {
      return equals(fstr);
    }
    bool operator != (const String &rhs) const
    {
      return !equals(rhs);
    }
    bool operator != (const char *cstr) const
    {
      return !equals(cstr);
    }
    bool operator < (const String &rhs) const
    {
      return compareTo(rhs) < 0;
    }
    bool operator > (const String &rhs) const
    {
      return compareTo(rhs) > 0;
    }
    bool operator <= (const String &rhs) const
	{
	  return compareTo(rhs) <= 0;
	}
    bool operator >= (const String &rhs) const
    {
      return compareTo(rhs) >= 0;
    }
    bool equalsIgnoreCase(const char* cstr) const;
    bool equalsIgnoreCase(const char* cstr, size_t length) const;
    bool equalsIgnoreCase(const String &s2) const
    {
    	return equalsIgnoreCase(s2.cbuffer(), s2.length());
    }
    bool equalsIgnoreCase(const FlashString& fstr) const;
    bool startsWith(const String &prefix) const
    {
    	return startsWith(prefix, 0);
    }
    bool startsWith(const String &prefix, size_t offset) const;
    bool endsWith(const String &suffix) const;

    // character acccess
    char charAt(size_t index) const
    {
      return operator[](index);
    }
    void setCharAt(size_t index, char c);
    char operator [](size_t index) const;
    char& operator [](size_t index);

    /** @brief read contents of string into a buffer
     *  @param buf buffer to write data
     *  @param bufsize size of buffer in bytes
     *  @param index offset to start
     *  @retval unsigned number of bytes copied, excluding nul terminator
     *  @note Returned data always nul terminated so buffer size needs to take this
     *  into account
     */
    size_t getBytes(unsigned char *buf, size_t bufsize, size_t index = 0) const;

    void toCharArray(char *buf, size_t bufsize, size_t index = 0) const
    {
      getBytes((unsigned char *)buf, bufsize, index);
    }
    const char* c_str() const { return cbuffer() ?: empty.cbuffer(); }
    char* begin() { return buffer(); }
    char* end() { return buffer() + length(); }
    const char* begin() const { return c_str(); }
    const char* end() const { return c_str() + length(); }
  
    // search
    int indexOf(char ch, size_t fromIndex = 0) const;
    int indexOf(const char* s2_buf, size_t fromIndex = 0, size_t s2_len = 0) const;
    int indexOf(const String &s2, size_t fromIndex = 0) const
    {
    	return indexOf(s2.cbuffer(), fromIndex, s2.length());
    }
    int lastIndexOf(char ch) const;
    int lastIndexOf(char ch, size_t fromIndex) const;
    int lastIndexOf(const String &s2) const;
    int lastIndexOf(const String &s2, size_t fromIndex) const;
    int lastIndexOf(const char* s2_buf, size_t fromIndex, size_t s2_len) const;
    String substring(size_t beginIndex) const { return substring(beginIndex, length()); }
    String substring(size_t beginIndex, size_t endIndex) const;

    // modification
    void replace(char find, char replace);
    bool replace(const String& find, const String& replace);
    bool replace(const char* find_buf, size_t find_len, const char* replace_buf, size_t replace_len);
    void remove(size_t index)
    {
    	remove(index, SIZE_MAX);
    }
    void remove(size_t index, size_t count);
    void toLowerCase(void);
    void toUpperCase(void);
    void trim(void);

    // parsing/conversion
    long toInt(void) const;
    float toFloat(void) const;

	/// Max chars. (excluding NUL terminator) we can store in SSO mode
	static constexpr size_t SSO_CAPACITY = STRING_OBJECT_SIZE - 2;

protected:
    /// Used when contents allocated on heap
	struct PtrBuf {
		char* buffer;		  // the actual char array
		size_t len;			  // the String length (not counting the '\0')
		size_t capacity : 31; // the array length minus one (for the '\0')
	};
	// For small strings we can store data directly without requiring the heap
	struct SsoBuf {
		char buffer[SSO_CAPACITY + 1];
		unsigned char len : 7;
		unsigned char set : 1; ///< true for SSO mode
	};
	union {
		struct {
			size_t u32[STRING_OBJECT_SIZE / 4] = {0};
		};
		PtrBuf ptr;
		SsoBuf sso;
	};

	static_assert(STRING_OBJECT_SIZE == sizeof(SsoBuf), "SSO Buffer alignment problem");
	static_assert(STRING_OBJECT_SIZE >= sizeof(PtrBuf), "STRING_OBJECT_SIZE too small");
	static_assert(STRING_OBJECT_SIZE <= 128, "STRING_OBJECT_SIZE too large (max. 128)");
	static_assert(STRING_OBJECT_SIZE % 4 == 0, "STRING_OBJECT_SIZE must be a multiple of 4");

protected:
	// Free any heap memory and set to non-SSO mode; isNull() will return true
	void invalidate(void);

    // String is Null (invalid) by default, i.e. non-SSO and null buffer
    __forceinline bool isNull() const
    {
    	return !sso.set && (ptr.buffer == nullptr);
    }

    // Get writeable buffer pointer
    __forceinline char* buffer()
    {
    	return sso.set ? sso.buffer : ptr.buffer;
    }

    // Get read-only buffer pointer
    __forceinline const char* cbuffer() const
    {
    	return sso.set ? sso.buffer : ptr.buffer;
    }

    // Get currently assigned capacity for current mode
    __forceinline size_t capacity() const
    {
    	return sso.set ? SSO_CAPACITY : ptr.capacity;
    }

    // Called whenever string length changes to ensure NUL terminator is set
    __forceinline void setlen(size_t len)
    {
    	if(sso.set) {
    		sso.len = len;
    		sso.buffer[len] = '\0';
    	} else {
    		ptr.len = len;
    		ptr.buffer[len] = '\0';
    	}
    }

    // copy and move
    String & copy(const char *cstr, size_t length);
    String& copy(flash_string_t pstr, size_t length);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    void move(String &rhs);
#endif
};

class StringSumHelper : public String
{
  public:
    StringSumHelper(const String &s) : String(s) {}
    StringSumHelper(const char *p) : String(p) {}
    StringSumHelper(char c) : String(c) {}
    StringSumHelper(unsigned char num) : String(num) {}
    StringSumHelper(int num) : String(num) {}
    StringSumHelper(unsigned int num) : String(num) {}
    StringSumHelper(long num) : String(num) {}
    StringSumHelper(long long num) : String(num) {}
    StringSumHelper(unsigned long num) : String(num) {}
    StringSumHelper(unsigned long long num) : String(num) {}
    StringSumHelper(float num) : String(num) {}
    StringSumHelper(double num) : String(num) {}
};

#include "FlashString.h"
#include "SplitString.h"

#endif  // __cplusplus
