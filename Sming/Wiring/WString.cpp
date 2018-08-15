/* $Id: WString.cpp 1156 2011-06-07 04:01:16Z bhagman $
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

#include "WiringFrameworkIncludes.h"

/*
 * A null string where functions needs to return a const reference
 */
const String String::nullstr = nullptr;
const String String::empty = "";

/*********************************************/
/*  Constructors                             */
/*********************************************/

String::String(const char* cstr)
{
	init();
	if (cstr)
		copy(cstr, strlen(cstr));
}

String::String(const char* cstr, unsigned length)
{
	init();
	if (cstr)
		copy(cstr, length);
}

String::String(const String& value)
{
	init();
	*this = value;
}

String::String(flash_string_t pstr, int length)
{
	init();
	setString(pstr, length);
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
String::String(String&& rval)
{
	init();
	move(rval);
}
String::String(StringSumHelper&& rval)
{
	init();
	move(rval);
}
#endif

String::String(char c)
{
	init();
	char buf[2];
	buf[0] = c;
	buf[1] = 0;
	*this = buf;
}

String::String(unsigned char value, unsigned char base)
{
	init();
	char buf[8 + 8 * sizeof(value)];
	ultoa(value, buf, base);
	*this = buf;
}

String::String(int value, unsigned char base)
{
	init();
	char buf[2 + 8 * sizeof(value)];
	itoa(value, buf, base);
	*this = buf;
}

String::String(unsigned value, unsigned char base)
{
	init();
	// Worst-case sized for binary output
	char buf[8 + 8 * sizeof(value)];
	ultoa(value, buf, base);
	*this = buf;
}

String::String(long value, unsigned char base)
{
	init();
	char buf[2 + 8 * sizeof(value)];
	ltoa(value, buf, base);
	*this = buf;
}

String::String(unsigned long value, unsigned char base)
{
	init();
	char buf[1 + 8 * sizeof(value)];
	ultoa(value, buf, base);
	*this = buf;
}

String::String(float value, unsigned char decimalPlaces)
{
	init();
	char buf[33];
	*this = dtostrf(value, 0, decimalPlaces, buf);
}

String::String(double value, unsigned char decimalPlaces)
{
	init();
	char buf[33];
	*this = dtostrf(value, 0, decimalPlaces, buf);
}

String::~String()
{
	if (_buffer)
		free(_buffer);
}

void String::setString(const char* cstr, int length)
{
	if (cstr) {
		if (length < 0)
			length = strlen(cstr);
		copy(cstr, length);
	}
}

void String::setString(flash_string_t pstr, int length)
{
	if (pstr) {
		if (length < 0)
			length = strlen_P((PGM_P)pstr);
		copy(pstr, length);
	}
}

/*********************************************/
/*  Memory Management                        */
/*********************************************/

inline void String::init(void)
{
	_buffer = nullptr;
	_capacity = 0;
	_len = 0;
	//flags = 0;
}

void String::invalidate(void)
{
	if (_buffer)
		free(_buffer);
	_buffer = nullptr;
	_capacity = _len = 0;
}

bool String::reserve(unsigned size)
{
	if (_buffer && _capacity >= size)
		return true;

	if (!changeBuffer(size))
		return false;

	if (_len == 0)
		_buffer[0] = '\0';
	return true;
}

bool String::setLength(unsigned size)
{
	if (!reserve(size))
		return false;

	_len = size;
	if (_buffer)
		_buffer[_len] = '\0';

	return true;
}

bool String::changeBuffer(unsigned maxStrLen)
{
	char* newbuffer = (char*)realloc(_buffer, maxStrLen + 1);
	if (!newbuffer)
		return false;

	_buffer = newbuffer;
	_capacity = maxStrLen;
	return true;
}

/*********************************************/
/*  Copy and Move                            */
/*********************************************/

String& String::copy(const char* cstr, unsigned length)
{
	if (!reserve(length)) {
		invalidate();
		return *this;
	}

	_len = length;
	memmove(_buffer, cstr, length);
	_buffer[_len] = '\0';
	return *this;
}

String& String::copy(flash_string_t pstr, unsigned length)
{
	// If necessary, allocate additional space so copy can be aligned
	unsigned length_aligned = ALIGNUP(length);
	if (!reserve(length_aligned)) {
		invalidate();
	}
	else {
		/* @todo if we can be sure that the heap allocator word-aligns _buffer then we can use
		 * memcpy_aligned; we use memcpy_P because it checks for this
		 */
		memcpy_P(_buffer, (PGM_P)pstr, length_aligned);
		_buffer[length] = '\0';
		_len = length;
	}
	return *this;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void String::move(String& rhs)
{
	if (_buffer)
		free(_buffer);
	_buffer = rhs._buffer;
	_capacity = rhs._capacity;
	_len = rhs._len;
	rhs._buffer = nullptr;
	rhs._capacity = 0;
	rhs._len = 0;
}
#endif

String& String::operator=(const String& rhs)
{
	if (this == &rhs)
		return *this;

	if (rhs._buffer)
		copy(rhs._buffer, rhs._len);
	else
		invalidate();

	return *this;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
String& String::operator=(String&& rval)
{
	if (this != &rval)
		move(rval);
	return *this;
}

String& String::operator=(StringSumHelper&& rval)
{
	if (this != &rval)
		move(rval);
	return *this;
}
#endif

String& String::operator=(const char* cstr)
{
	if (cstr)
		copy(cstr, strlen(cstr));
	else
		invalidate();

	return *this;
}

/*********************************************/
/*  concat                                   */
/*********************************************/

bool String::concat(const String& s)
{
	return concat(s._buffer, s._len);
}

bool String::concat(const char* cstr, unsigned length)
{
	unsigned newlen = _len + length;
	if (!cstr)
		return false;
	if (length == 0)
		return true;
	if (!reserve(newlen))
		return false;
	memmove(_buffer + _len, cstr, length);
	_len = newlen;
	_buffer[_len] = '\0';
	return true;
}

bool String::concat(const char* cstr)
{
	return cstr ? concat(cstr, strlen(cstr)) : false;
}

bool String::concat(char c)
{
	char buf[2];
	buf[0] = c;
	buf[1] = 0;
	return concat(buf, 1);
}

bool String::concat(unsigned char num)
{
	char buf[1 + 3 * sizeof(unsigned char)];
	itoa(num, buf, 10);
	return concat(buf, strlen(buf));
}

bool String::concat(int num)
{
	char buf[2 + 3 * sizeof(int)];
	itoa(num, buf, 10);
	return concat(buf, strlen(buf));
}

bool String::concat(unsigned num)
{
	char buf[8 + 3 * sizeof(unsigned)];
	ultoa(num, buf, 10);
	return concat(buf, strlen(buf));
}

bool String::concat(long num)
{
	char buf[2 + 3 * sizeof(long)];
	ltoa(num, buf, 10);
	return concat(buf, strlen(buf));
}

bool String::concat(unsigned long num)
{
	char buf[1 + 3 * sizeof(unsigned long)];
	ultoa(num, buf, 10);
	return concat(buf, strlen(buf));
}

bool String::concat(float num)
{
	char buf[20];
	char* string = dtostrf(num, 4, 2, buf);
	return concat(string, strlen(string));
}

bool String::concat(double num)
{
	char buf[20];
	char* string = dtostrf(num, 4, 2, buf);
	return concat(string, strlen(string));
}

/*********************************************/
/*  Concatenate                              */
/*********************************************/

StringSumHelper& operator+(const StringSumHelper& lhs, const String& rhs)
{
	StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(rhs._buffer, rhs._len))
		a.invalidate();
	return a;
}

StringSumHelper& operator+(const StringSumHelper& lhs, const char* cstr)
{
	StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
	if (!cstr || !a.concat(cstr, strlen(cstr)))
		a.invalidate();
	return a;
}

StringSumHelper& operator+(const StringSumHelper& lhs, char c)
{
	StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(c))
		a.invalidate();
	return a;
}

StringSumHelper& operator+(const StringSumHelper& lhs, unsigned char num)
{
	StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num))
		a.invalidate();
	return a;
}

StringSumHelper& operator+(const StringSumHelper& lhs, int num)
{
	StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num))
		a.invalidate();
	return a;
}

StringSumHelper& operator+(const StringSumHelper& lhs, unsigned num)
{
	StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num))
		a.invalidate();
	return a;
}

StringSumHelper& operator+(const StringSumHelper& lhs, long num)
{
	StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num))
		a.invalidate();
	return a;
}

StringSumHelper& operator+(const StringSumHelper& lhs, unsigned long num)
{
	StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num))
		a.invalidate();
	return a;
}

StringSumHelper& operator+(const StringSumHelper& lhs, float num)
{
	StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num))
		a.invalidate();
	return a;
}

StringSumHelper& operator+(const StringSumHelper& lhs, double num)
{
	StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num))
		a.invalidate();
	return a;
}

/*********************************************/
/*  Comparison                               */
/*********************************************/

int String::compareTo(const String& s) const
{
	if (!_buffer || !s._buffer) {
		if (s._buffer && s._len > 0)
			return 0 - *(unsigned char*)s._buffer;
		if (_buffer && _len > 0)
			return *(unsigned char*)_buffer;
		return 0;
	}
	return strcmp(_buffer, s._buffer);
}

bool String::equals(const String& s2) const
{
	return (_len == s2._len && compareTo(s2) == 0);
}

bool String::equals(const char* cstr) const
{
	if (_len == 0)
		return !cstr || *cstr == '\0';

	if (!cstr)
		return _buffer[0] == '\0';

	return strcmp(_buffer, cstr) == 0;
}

bool String::operator<(const String& rhs) const
{
	return compareTo(rhs) < 0;
}

bool String::operator>(const String& rhs) const
{
	return compareTo(rhs) > 0;
}

bool String::operator<=(const String& rhs) const
{
	return compareTo(rhs) <= 0;
}

bool String::operator>=(const String& rhs) const
{
	return compareTo(rhs) >= 0;
}

bool String::equalsIgnoreCase(const char* cstr) const
{
	if (_buffer == cstr)
		return true;

	return strcasecmp(cstr, _buffer) == 0;
}

bool String::equalsIgnoreCase(const String& s2) const
{
	if (_len != s2._len)
		return false;

	if (_len == 0)
		return true;

	return equalsIgnoreCase(s2._buffer);
}

bool String::startsWith(const String& s2) const
{
	if (_len < s2._len)
		return false;

	return startsWith(s2, 0);
}

bool String::startsWith(const String& s2, unsigned offset) const
{
	if (offset + s2._len > _len || !_buffer || !s2._buffer)
		return false;

	return strncmp(&_buffer[offset], s2._buffer, s2._len) == 0;
}

unsigned char String::endsWith(const String& s2) const
{
	if (_len < s2._len || !_buffer || !s2._buffer)
		return 0;

	return strcmp(&_buffer[_len - s2._len], s2._buffer) == 0;
}

/*********************************************/
/*  Character Access                         */
/*********************************************/

char String::charAt(unsigned loc) const
{
	return operator[](loc);
}

void String::setCharAt(unsigned loc, char c)
{
	if (loc < _len)
		_buffer[loc] = c;
}

char& String::operator[](unsigned index)
{
	if (index < _len)
		return _buffer[index];

	static char dummy_writable_char;
	dummy_writable_char = '\0';
	return dummy_writable_char;
}

char String::operator[](unsigned index) const
{
	return (index < _len) ? _buffer[index] : '\0';
}

unsigned String::getBytes(unsigned char* buf, unsigned bufsize, unsigned index) const
{
	if (bufsize == 0 || !buf)
		return 0;

	if (index >= _len) {
		buf[0] = '\0';
		return 0;
	}

	unsigned n = bufsize - 1;
	if (n > _len - index)
		n = _len - index;
	memmove(buf, _buffer + index, n);
	buf[n] = '\0';
	return n;
}

/*********************************************/
/*  Search                                   */
/*********************************************/

int String::indexOf(char c) const
{
	return indexOf(c, 0);
}

int String::indexOf(char ch, unsigned fromIndex) const
{
	if (fromIndex >= _len)
		return -1;

	const char* temp = strchr(_buffer + fromIndex, ch);
	return temp ? (temp - _buffer) : -1;
}

int String::indexOf(const String& s2) const
{
	return indexOf(s2, 0);
}

int String::indexOf(const String& s2, unsigned fromIndex) const
{
	if (fromIndex >= _len)
		return -1;

	const char* found = strstr(_buffer + fromIndex, s2._buffer);
	return found ? (found - _buffer) : -1;
}

int String::lastIndexOf(char theChar) const
{
	return lastIndexOf(theChar, _len - 1);
}

int String::lastIndexOf(char ch, unsigned fromIndex) const
{
	if (fromIndex >= _len)
		return -1;

	char tempchar = _buffer[fromIndex + 1];
	_buffer[fromIndex + 1] = '\0';
	char* temp = strrchr(_buffer, ch);
	_buffer[fromIndex + 1] = tempchar;
	return temp ? (temp - _buffer) : -1;
}

int String::lastIndexOf(const String& s2) const
{
	return lastIndexOf(s2, _len - s2._len);
}

int String::lastIndexOf(const String& s2, unsigned fromIndex) const
{
	if (s2._len == 0 || _len == 0 || s2._len > _len)
		return -1;

	if (fromIndex >= _len)
		fromIndex = _len - 1;

	int found = -1;
	for (char* p = _buffer; p <= _buffer + fromIndex; p++) {
		p = strstr(p, s2._buffer);
		if (!p)
			break;
		if (p <= _buffer + fromIndex)
			found = p - _buffer;
	}

	return found;
}

String String::substring(unsigned left, unsigned right) const
{
	if (!_buffer)
		return nullptr;

	if (left > right) {
		unsigned temp = right;
		right = left;
		left = temp;
	}
	String out;
	if (left > _len)
		return out;
	if (right > _len)
		right = _len;
	char temp = _buffer[right]; // save the replaced character
	_buffer[right] = '\0';
	out = _buffer + left;  // pointer arithmetic
	_buffer[right] = temp; //restore character
	return out;
}

/*********************************************/
/*  Modification                             */
/*********************************************/

void String::replace(char find, char replace)
{
	if (!_buffer)
		return;

	for (char* p = _buffer; *p; p++)
		if (*p == find)
			*p = replace;
}

void String::replace(const String& find, const String& replace)
{
	if (_len == 0 || find._len == 0)
		return;
	int diff = replace._len - find._len;
	char* readFrom = _buffer;
	char* foundAt;
	if (diff == 0) {
		while ((foundAt = strstr(readFrom, find._buffer))) {
			memmove(foundAt, replace._buffer, replace._len);
			readFrom = foundAt + replace._len;
		}
	}
	else if (diff < 0) {
		char* writeTo = _buffer;
		while ((foundAt = strstr(readFrom, find._buffer))) {
			unsigned n = foundAt - readFrom;
			memcpy(writeTo, readFrom, n);
			writeTo += n;
			memmove(writeTo, replace._buffer, replace._len);
			writeTo += replace._len;
			readFrom = foundAt + find._len;
			_len += diff;
		}
		strcpy(writeTo, readFrom);
	}
	else {
		unsigned size = _len; // compute size needed for result
		while ((foundAt = strstr(readFrom, find._buffer))) {
			readFrom = foundAt + find._len;
			size += diff;
		}
		if (size == _len)
			return;
		if (size > _capacity && !changeBuffer(size))
			return; // XXX: tell user!
		int index = _len - 1;
		while ((index = lastIndexOf(find, index)) >= 0) {
			readFrom = _buffer + index + find._len;
			memmove(readFrom + diff, readFrom, _len - (readFrom - _buffer));
			_len += diff;
			memmove(_buffer + index, replace._buffer, replace._len);
			index--;
		}
		_buffer[_len] = '\0';
	}
}

void String::remove(unsigned index)
{
	if (index < _len)
		remove(index, _len - index);
}

void String::remove(unsigned index, unsigned count)
{
	if (index >= _len || count <= 0)
		return;

	if (index + count > _len)
		count = _len - index;
	_len -= count;
	memmove(_buffer + index, _buffer + index + count, _len - index);
	_buffer[_len] = '\0';
}

void String::toLowerCase(void)
{
	if (_buffer)
		for (char* p = _buffer; *p; p++)
			*p = tolower(*p);
}

void String::toUpperCase(void)
{
	if (_buffer)
		for (char* p = _buffer; *p; p++)
			*p = toupper(*p);
}

void String::trim(void)
{
	if (!_buffer || _len == 0)
		return;

	char* begin = _buffer;
	while (isspace(*begin))
		begin++;
	char* end = _buffer + _len - 1;
	while (isspace(*end) && end >= begin)
		end--;
	_len = end + 1 - begin;
	if (begin > _buffer)
		memmove(_buffer, begin, _len);
	_buffer[_len] = 0;
}

/*********************************************/
/*  Parsing / Conversion                     */
/*********************************************/

long String::toInt(void) const
{
	return _buffer ? atoi(_buffer) : 0;
}

float String::toFloat(void) const
{
	return _buffer ? atof(_buffer) : 0;
}

/** @brief Encode binary data as array of hex charactres
 *  @param data Data to encode
 *  @param length Number of data bytes to encode
 *  @param sep Optional separator between byte characters
 *  @retval true on success, false on memory allocation error
 *  @note Uses 2 characters per byte - always
 */
String toHexString(const uint8_t* data, unsigned sz, char sep)
{
	String s;

	unsigned nc = sep ? 3 : 2;
	if (!s.setLength(nc * sz + 1))
		return nullptr;

	char* p = s.begin();
	for (unsigned i = 0; i < sz; ++i) {
		if (i && sep)
			*p++ = sep;
		*p++ = hexchar(data[i] >> 4);
		*p++ = hexchar(data[i] & 0x0F);
	}
	*p = '\0';

	return s;
}

/*
 * August 2018 (mikee47)
 *
 * 	splitString moved out of its own module. Does not require friend access.
 * 	what parameter is now const, and is NOT trimmed prior to parsing.
 * 	Caller should do that if required.
 * 	WARNING: using whitespace separators should be avoided as multiple
 * 	separators will produce multiple empty strings.
 *
 * Question: Should this be a member function of String ?
 * String.split(delim, splits)
 *
 * Answer: Yes.
 *
 */
unsigned String::split(char delim, Vector<String>& splits) const
{
	splits.removeAllElements();
	unsigned splitCount = 0;
	unsigned splitIndex = 0;
	unsigned startIndex = 0;
	for (unsigned i = 0; i < _len; i++)
		if (_buffer[i] == delim) {
			splits.addElement(substring(startIndex, i));
			splitIndex++;
			startIndex = i + 1;
			splitCount++;
		}

	splits.addElement(substring(startIndex, _len));

	return splitCount + 1;
}
