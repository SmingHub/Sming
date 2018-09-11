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

#ifndef WSTRING_H
#define WSTRING_H

#ifdef __cplusplus

#include "WiringFrameworkDependencies.h"
#include "WVector.h"

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

// The string class
class String
{
	// use a function pointer to allow for "if (s)" without the
	// complications of an operator bool(). for more information, see:
	// http://www.artima.com/cppsource/safebool.html
	typedef void (String::*StringIfHelperType)() const;
	void IRAM_ATTR StringIfHelper() const
	{
	}

public:
	// constructors
	// creates a copy of the initial value.
	// if the initial value is null or invalid, or if memory allocation
	// fails, the string will be marked as invalid (i.e. "if (s)" will
	// be false).
	IRAM_ATTR String(const char* cstr = "");
	IRAM_ATTR String(const char* cstr, unsigned int length);
	IRAM_ATTR String(const String& str);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	IRAM_ATTR String(String&& rval);
	IRAM_ATTR String(StringSumHelper&& rval);
#endif
	explicit String(char c);
	explicit String(unsigned char, unsigned char base = 10);
	explicit String(int, unsigned char base = 10);
	explicit String(unsigned int, unsigned char base = 10);
	explicit String(long, unsigned char base = 10);
	explicit String(unsigned long, unsigned char base = 10);
	explicit String(float, unsigned char decimalPlaces = 2);
	explicit String(double, unsigned char decimalPlaces = 2);
	~String(void);

	void setString(const char* cstr, int length = -1);

	// memory management
	// return true on success, false on failure (in which case, the string
	// is left unchanged).  reserve(0), if successful, will validate an
	// invalid string (i.e., "if (s)" will be true afterwards)
	unsigned char reserve(unsigned int size);

	/** @brief set the string length accordingly, expanding if necessary
	 *  @param length required for string (nul terminator additional)
	 *  @retval true on success, false on failure
	 *  @note extra characters are undefined
	 */
	bool setLength(unsigned length);

	inline unsigned int length(void) const
	{
		return len;
	}

	// creates a copy of the assigned value.  if the value is null or
	// invalid, or if the memory allocation fails, the string will be
	// marked as invalid ("if (s)" will be false).
	String& IRAM_ATTR operator=(const String& rhs);
	String& IRAM_ATTR operator=(const char* cstr);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	String& operator=(String&& rval);
	String& operator=(StringSumHelper&& rval);
#endif

	// concatenate (works w/ built-in types)

	// returns true on success, false on failure (in which case, the string
	// is left unchanged).  if the argument is null or invalid, the
	// concatenation is considered unsucessful.
	unsigned char concat(const String& str);
	unsigned char concat(const char* cstr);
	unsigned char concat(char c);
	unsigned char concat(unsigned char c);
	unsigned char concat(int num);
	unsigned char concat(unsigned int num);
	unsigned char concat(long num);
	unsigned char concat(unsigned long num);
	unsigned char concat(float num);
	unsigned char concat(double num);

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
	String& operator+=(unsigned int num)
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
	friend StringSumHelper& operator+(const StringSumHelper& lhs, unsigned int num);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, long num);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, unsigned long num);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, float num);
	friend StringSumHelper& operator+(const StringSumHelper& lhs, double num);

	// comparison (only works w/ Strings and "strings")
	operator StringIfHelperType() const
	{
		return buffer ? &String::StringIfHelper : 0;
	}
	int IRAM_ATTR compareTo(const String& s) const;
	unsigned char IRAM_ATTR equals(const String& s) const;
	unsigned char IRAM_ATTR equals(const char* cstr) const;
	unsigned char IRAM_ATTR operator==(const String& rhs) const
	{
		return equals(rhs);
	}
	unsigned char IRAM_ATTR operator==(const char* cstr) const
	{
		return equals(cstr);
	}
	unsigned char IRAM_ATTR operator!=(const String& rhs) const
	{
		return !equals(rhs);
	}
	unsigned char IRAM_ATTR operator!=(const char* cstr) const
	{
		return !equals(cstr);
	}
	unsigned char operator<(const String& rhs) const;
	unsigned char operator>(const String& rhs) const;
	unsigned char operator<=(const String& rhs) const;
	unsigned char operator>=(const String& rhs) const;
	unsigned char equalsIgnoreCase(const String& s) const;
	unsigned char startsWith(const String& prefix) const;
	unsigned char startsWith(const String& prefix, unsigned int offset) const;
	unsigned char endsWith(const String& suffix) const;

	// character acccess
	char IRAM_ATTR charAt(unsigned int index) const;
	void IRAM_ATTR setCharAt(unsigned int index, char c);
	char IRAM_ATTR operator[](unsigned int index) const;
	char& IRAM_ATTR operator[](unsigned int index);
	void getBytes(unsigned char* buf, unsigned int bufsize, unsigned int index = 0) const;
	void toCharArray(char* buf, unsigned int bufsize, unsigned int index = 0) const
	{
		getBytes((unsigned char*)buf, bufsize, index);
	}
	const char* c_str() const
	{
		return buffer;
	}
	char* begin()
	{
		return buffer;
	}
	char* end()
	{
		return buffer + length();
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
	int IRAM_ATTR indexOf(char ch) const;
	int indexOf(char ch, unsigned int fromIndex) const;
	int IRAM_ATTR indexOf(const String& str) const;
	int indexOf(const String& str, unsigned int fromIndex) const;
	int lastIndexOf(char ch) const;
	int lastIndexOf(char ch, int fromIndex) const;
	int lastIndexOf(const String& str) const;
	int lastIndexOf(const String& str, int fromIndex) const;
	String substring(unsigned int beginIndex) const
	{
		return substring(beginIndex, len);
	};
	String substring(unsigned int beginIndex, unsigned int endIndex) const;

	// modification
	void replace(char find, char replace);
	void replace(const String& find, const String& replace);
	void remove(unsigned int index);
	void remove(unsigned int index, unsigned int count);
	void toLowerCase(void);
	void toUpperCase(void);
	void trim(void);

	// parsing/conversion
	long toInt(void) const;
	float toFloat(void) const;

	friend int splitString(String& what, int delim, Vector<long>& splits);
	friend int splitString(String& what, int delim, Vector<int>& splits);
	friend int splitString(String& what, int delim, Vector<String>& splits);

	//void printTo(Print &p) const;

protected:
	char* buffer;	  // the actual char array
	uint16_t capacity; // the array length minus one (for the '\0')
	uint16_t len;	  // the String length (not counting the '\0')
					   //unsigned char flags;    // unused, for future features
protected:
	void IRAM_ATTR init(void);
	void IRAM_ATTR invalidate(void);
	unsigned char IRAM_ATTR changeBuffer(unsigned int maxStrLen);
	unsigned char IRAM_ATTR concat(const char* cstr, unsigned int length);

	// copy and move
	String& copy(const char* cstr, unsigned int length);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	void move(String& rhs);
#endif
};

class StringSumHelper : public String
{
public:
	StringSumHelper(const String& s) : String(s)
	{
	}
	StringSumHelper(const char* p) : String(p)
	{
	}
	StringSumHelper(char c) : String(c)
	{
	}
	StringSumHelper(unsigned char num) : String(num)
	{
	}
	StringSumHelper(int num) : String(num)
	{
	}
	StringSumHelper(unsigned int num) : String(num)
	{
	}
	StringSumHelper(long num) : String(num)
	{
	}
	StringSumHelper(unsigned long num) : String(num)
	{
	}
	StringSumHelper(float num) : String(num)
	{
	}
	StringSumHelper(double num) : String(num)
	{
	}
};

#endif // __cplusplus
#endif
// WSTRING_H
