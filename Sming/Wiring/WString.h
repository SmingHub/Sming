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
 * 12/8/2018 mikee47
 *
 * 	Reformatted to coding style
 * 	Member variables prefixed with underscore
 * 	String::nullstr and String::empty added
 * 	Support added for flash strings
 * 	null check added before freeing buffer in destructor
 * 	'unsigned char' replaced with 'bool' where appropriate
 *	memmove used exclusively for copy operations to support non-ASCII data in Strings
 *  getBytes() now returns size
 *  split() method added
 *  toHexString function added
 *  String methods not in IRAM by default; if required, #define STRING_IRAM_ATTR
 *   	(note: String or any object access in ISR is bad idea; defer it instead).
 *  substring() method missing null buffer check
 *
 */

#ifndef WSTRING_H
#define WSTRING_H

#ifdef __cplusplus

#include "WiringFrameworkDependencies.h"
#include "WVector.h"

#define STRING_IRAM_ATTR // IRAM_ATTR

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

// an abstract class used as a means to proide a unique pointer type
// but really has no body
class __FlashStringHelper;
typedef const __FlashStringHelper* flash_string_t;
#define FPSTR(pstr_pointer) reinterpret_cast<flash_string_t>(pstr_pointer)
#define F(string_literal) String(FPSTR(PSTR(string_literal)), sizeof(string_literal) - 1)

// Define a flash string, accessed wrapped in a function call _name()
#define DEFINE_STRING_P(_name, _str)                                                                                   \
	String _name()                                                                                                     \
	{                                                                                                                  \
		static PSTR_DEFINE(s, _str) return String(FPSTR(s), sizeof(_str) - 1);                                         \
	}

// Declare a global reference to _name()
#define DECLARE_STRING_P(_name) extern String _name();

#define String_P(__flashstr) String((flash_string_t)__flashstr, -1)

/**
 * @brief The string class
 *
 * Note that a string object's default constructor creates an empty string.
 * This is not the same as a null string.
 * A null string evaluates to false
 * An empty string evaluates to true
 */
// The string class
class String {
	// use a function pointer to allow for "if (s)" without the
	// complications of an operator bool(). for more information, see:
	// http://www.artima.com/cppsource/safebool.html
	typedef void (String::*StringIfHelperType)() const;
	void STRING_IRAM_ATTR StringIfHelper() const
	{}

public:
	static const String nullstr;
	static const String empty;

	/*
	 constructors

	 creates a copy of the initial value.
	 if the initial value is null or invalid, or if memory allocation
	 fails, the string will be marked as invalid, thus "if (s)" will
	 fail.
	 */
	STRING_IRAM_ATTR
	String(const char* cstr = nullptr);
	STRING_IRAM_ATTR
	String(const char* cstr, unsigned length);
	STRING_IRAM_ATTR
	String(const String& str);
	String(flash_string_t pstr, int length);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	STRING_IRAM_ATTR
	String(String&& rval);
	STRING_IRAM_ATTR
	String(StringSumHelper&& rval);
#endif
	explicit String(char c);
	explicit String(unsigned char, unsigned char base = 10);
	explicit String(int, unsigned char base = 10);
	explicit String(unsigned, unsigned char base = 10);
	explicit String(long, unsigned char base = 10);
	explicit String(unsigned long, unsigned char base = 10);
	explicit String(float, unsigned char decimalPlaces = 2);
	explicit String(double, unsigned char decimalPlaces = 2);
	~String(void);

	void setString(const char* cstr, int length = -1);
	void setString(flash_string_t pstr, int length = -1);

	// memory management
	// return true on success, false on failure (in which case, the string
	// is left unchanged).  reserve(0), if successful, will validate an
	// invalid string (i.e., "if (s)" will be true afterwards)
	bool reserve(unsigned size);
	/** @brief set the string length accordingly, expanding if necessary
	 *  @param length required for string (nul terminator additional)
	 *  @retval true on success, false on failure
	 *  @note extra characters are undefined
	 */
	bool setLength(unsigned length);

	unsigned length(void) const
	{
		return _len;
	}

	// creates a copy of the assigned value.  if the value is null or
	// invalid, or if the memory allocation fails, the string will be
	// marked as invalid ("if (s)" will be false).
	String& STRING_IRAM_ATTR operator=(const String& rhs);
	String& STRING_IRAM_ATTR operator=(const char* cstr);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	String& operator=(String&& rval);
	String& operator=(StringSumHelper&& rval);
#endif

	// concatenate (works w/ built-in types)

	// returns true on success, false on failure (in which case, the string
	// is left unchanged).  if the argument is null or invalid, the
	// concatenation is considered unsucessful.
	bool concat(const String& str);
	bool concat(const char* cstr);
	bool STRING_IRAM_ATTR concat(const char* cstr, unsigned length);
	bool concat(char c);
	bool concat(unsigned char c);
	bool concat(int num);
	bool concat(unsigned num);
	bool concat(long num);
	bool concat(unsigned long num);
	bool concat(float num);
	bool concat(double num);

	// if there's not enough memory for the concatenated value, the string
	// will be left unchanged (but this isn't signalled in any way)
	String& operator+=(const String& rhs)
	{
		concat(rhs);
		return (*this);
	}
	String& operator+=(const char* cstr)
	{
		concat(cstr);
		return (*this);
	}
	String& operator+=(char c)
	{
		concat(c);
		return (*this);
	}
	String& operator+=(unsigned char num)
	{
		concat(num);
		return (*this);
	}
	String& operator+=(int num)
	{
		concat(num);
		return (*this);
	}
	String& operator+=(unsigned num)
	{
		concat(num);
		return (*this);
	}
	String& operator+=(long num)
	{
		concat(num);
		return (*this);
	}
	String& operator+=(unsigned long num)
	{
		concat(num);
		return (*this);
	}
	String& operator+=(float num)
	{
		concat(num);
		return (*this);
	}
	String& operator+=(double num)
	{
		concat(num);
		return (*this);
	}

	friend StringSumHelper& operator+(const StringSumHelper& lhs, const String& rhs);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, const char* cstr);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, char c);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, unsigned char num);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, int num);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, unsigned num);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, long num);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, unsigned long num);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, float num);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, double num);

	// comparison (only works w/ Strings and "strings")
	operator StringIfHelperType() const
	{
		return _buffer ? &String::StringIfHelper : 0;
	}
	int STRING_IRAM_ATTR compareTo(const String& s) const;
	bool STRING_IRAM_ATTR equals(const String& s) const;
	bool STRING_IRAM_ATTR equals(const char* cstr) const;
	bool STRING_IRAM_ATTR operator==(const String& rhs) const
	{
		return equals(rhs);
	}
	bool STRING_IRAM_ATTR operator==(const char* cstr) const
	{
		return equals(cstr);
	}
	bool STRING_IRAM_ATTR operator!=(const String& rhs) const
	{
		return !equals(rhs);
	}
	bool STRING_IRAM_ATTR operator!=(const char* cstr) const
	{
		return !equals(cstr);
	}
	bool operator<(const String& rhs) const;
	bool operator>(const String& rhs) const;
	bool operator<=(const String& rhs) const;
	bool operator>=(const String& rhs) const;
	bool equalsIgnoreCase(const char* cstr) const;
	bool equalsIgnoreCase(const String& s) const;
	bool startsWith(const String& prefix) const;
	bool startsWith(const String& prefix, unsigned offset) const;
	unsigned char endsWith(const String& suffix) const;

	// character acccess
	char STRING_IRAM_ATTR charAt(unsigned index) const;
	void STRING_IRAM_ATTR setCharAt(unsigned index, char c);
	char STRING_IRAM_ATTR operator[](unsigned index) const;
	char& STRING_IRAM_ATTR operator[](unsigned index);

	/** @brief read contents of string into a buffer
	 *  @param buf buffer to write data
	 *  @param bufsize size of buffer in bytes
	 *  @param index offset to start
	 *  @retval unsigned number of bytes copied, excluding nul terminator
	 *  @note Returned data always nul terminated so buffer size needs to take this
	 *  into account
	 */
	unsigned getBytes(unsigned char* buf, unsigned bufsize, unsigned index = 0) const;
	void toCharArray(char* buf, unsigned bufsize, unsigned index = 0) const
	{
		getBytes((unsigned char*)buf, bufsize, index);
	}
	const char* c_str() const
	{
		return _buffer;
	}
	char* begin()
	{
		return _buffer;
	}
	char* end()
	{
		return _buffer + length();
	}
	const char* begin() const
	{
		return c_str();
	}
	const char* end() const
	{
		return c_str() + length();
	}

	// search
	int STRING_IRAM_ATTR indexOf(char ch) const;
	int indexOf(char ch, unsigned fromIndex) const;
	int STRING_IRAM_ATTR indexOf(const String& str) const;
	int indexOf(const String& str, unsigned fromIndex) const;
	int lastIndexOf(char ch) const;
	int lastIndexOf(char ch, unsigned fromIndex) const;
	int lastIndexOf(const String& str) const;
	int lastIndexOf(const String& str, unsigned fromIndex) const;
	String substring(unsigned beginIndex) const
	{
		return substring(beginIndex, _len);
	}
	String substring(unsigned beginIndex, unsigned endIndex) const;

	// modification
	void replace(char find, char replace);
	void replace(const String& find, const String& replace);
	void remove(unsigned index);
	void remove(unsigned index, unsigned count);
	void toLowerCase(void);
	void toUpperCase(void);
	void trim(void);

	// parsing/conversion
	long toInt(void) const;
	float toFloat(void) const;

	/** @brief Split a string into segments separated by a single delimiter character
	 * 	@param what the string to parse
	 * 	@param delim the separating character between segments
	 * 	@param splits the vector of strings to populate - it is cleared first
	 * 	@retval unsigned number of strings returned in splits
	 * 	@note was splitString() function
	 */
	unsigned split(char delim, Vector<String>& splits) const;

protected:
	char* _buffer;		// the actual char array
	uint16_t _capacity; // the array length minus one (for the '\0')
	uint16_t _len;		// the String length (not counting the '\0')
						//unsigned char flags;    // unused, for future features
protected:
	void STRING_IRAM_ATTR init(void);
	void STRING_IRAM_ATTR invalidate(void);
	bool STRING_IRAM_ATTR changeBuffer(unsigned maxStrLen);

	// copy and move
	String& copy(const char* cstr, unsigned length);
	String& copy(flash_string_t pstr, unsigned length);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	void move(String& rhs);
#endif
};

class StringSumHelper : public String {
public:
	StringSumHelper(const String& s) : String(s)
	{}
	StringSumHelper(const char* p) : String(p)
	{}
	StringSumHelper(char c) : String(c)
	{}
	StringSumHelper(unsigned char num) : String(num)
	{}

	StringSumHelper(int num) : String(num)
	{}
	StringSumHelper(unsigned num) : String(num)
	{}
	StringSumHelper(long num) : String(num)
	{}
	StringSumHelper(unsigned long num) : String(num)
	{}
	StringSumHelper(float num) : String(num)
	{}
	StringSumHelper(double num) : String(num)
	{}
};

String toHexString(const uint8_t* data, unsigned sz, char sep = '\0');

#endif // __cplusplus
#endif
// WSTRING_H
