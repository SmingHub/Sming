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

#include "WString.h"
#include <string.h>
#include <stringutil.h>
#include <stringconversion.h>
#include <assert.h>

const String String::nullstr = nullptr;
const String String::empty = "";

/*********************************************/
/*  Constructors                             */
/*********************************************/

String::String(const char *cstr)
{
  if (cstr) copy(cstr, strlen(cstr));
}

String::String(const FlashString& fstr)
{
  setString(fstr.data(), fstr.length());
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
String::String(StringSumHelper &&rval)
{
  move(rval);
}
#endif

String::String(char c)
{
  if (setLength(1))
	  buffer()[0] = c;
}

String::String(unsigned char value, unsigned char base)
{
  char buf[8 + 8 * sizeof(value)];
  ultoa(value, buf, base);
  *this = buf;
}

String::String(int value, unsigned char base)
{
  char buf[8 + 8 * sizeof(value)];
  itoa(value, buf, base);
  *this = buf;
}

String::String(unsigned int value, unsigned char base)
{
  char buf[8 + 8 * sizeof(value)];
  ultoa(value, buf, base);
  *this = buf;
}

String::String(long value, unsigned char base)
{
  char buf[8 + 8 * sizeof(value)];
  ltoa(value, buf, base);
  *this = buf;
}

String::String(long long value, unsigned char base)
{
  char buf[8 + 8 * sizeof(value)];
  lltoa(value, buf, base);
  *this = buf;
}

String::String(unsigned long value, unsigned char base)
{
  char buf[8 + 8 * sizeof(value)];
  ultoa(value, buf, base);
  *this = buf;
}

String::String(unsigned long long value, unsigned char base)
{
  char buf[8 + 8 * sizeof(value)];
  ulltoa(value, buf, base);
  *this = buf;
}

String::String(float value, unsigned char decimalPlaces)
{
	char buf[33];
	*this = dtostrf(value, 0, decimalPlaces, buf);
}

String::String(double value, unsigned char decimalPlaces)
{
	char buf[33];
	*this = dtostrf(value, 0, decimalPlaces, buf);
}

void String::setString(const char *cstr, int length /* = -1 */)
{
	if (cstr)
	{
		if (length < 0)
			length = strlen(cstr);
		copy(cstr, length);
	}
	else
	{
		invalidate();
	}
}

void String::setString(flash_string_t pstr, int length /* = -1 */)
{
	if(pstr)
	{
		if(length < 0)
			length = strlen_P((PGM_P)pstr);
		copy(pstr, length);
	}
	else
	{
		invalidate();
	}
}
/*********************************************/
/*  Memory Management                        */
/*********************************************/

void String::invalidate(void)
{
  if (sso.set) {
	  sso.set = false;
  } else {
	  free(ptr.buffer);
  }
  ptr.buffer = nullptr;
  ptr.capacity = ptr.len = 0;
}

bool String::setLength(size_t size)
{
	if(!reserve(size)) {
		return false;
	}

	setlen(size);
	return true;
}

bool String::reserve(size_t size)
{
	// Can we use SSO here to avoid allocation?
	if(size <= SSO_CAPACITY) {
		// If already using SSO then no further action required
		if(sso.set) {
			return true;
		}

		// If heap hasn't been used yet then switch to SSO mode
		if(ptr.buffer == nullptr) {
			sso.set = true;
		} else {
			// Otherwise continue with existing heap allocation
			assert(ptr.capacity >= size);
		}

		return true;
	}

	// Reallocation required?
	if(!sso.set && ptr.buffer != nullptr && ptr.capacity >= size) {
		return true; // Nope :-)
	}

	// Need to handle resizing an existing heap buffer and moving from SSO to heap
	char* newbuffer = (char*)realloc(sso.set ? nullptr : ptr.buffer, size + 1);
	if(newbuffer == nullptr) {
		// allocation failed - leave existing buffer arrangement unchanged
		return false;
	}

	if(sso.set) {
		// Move content out of SSO
		memcpy(newbuffer, sso.buffer, sso.len);
		ptr.len = sso.len;
		sso.set = false;
	}
	ptr.buffer = newbuffer;
	ptr.capacity = size;
	return true;
}

/*********************************************/
/*  Copy and Move                            */
/*********************************************/

String & String::copy(const char *cstr, size_t length)
{
  if (!reserve(length))
  {
    invalidate();
    return *this;
  }
  memmove(buffer(), cstr, length);
  setlen(length);
  return *this;
}

String &String::copy(flash_string_t pstr, size_t length)
{
	// If necessary, allocate additional space so copy can be aligned
	size_t length_aligned = ALIGNUP(length);
	if(!reserve(length_aligned))
	{
		invalidate();
	}
	else
	{
		memcpy_aligned(buffer(), (PGM_P)pstr, length);
		setlen(length);
	}
	return *this;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void String::move(String &rhs)
{
	if(rhs.isNull()) {
		invalidate();
		return;
	}

	auto rhs_len = rhs.length();
	if(rhs.sso.set) {
		// Switch to SSO if required
		reserve(rhs_len);
	}

	// If we already have capacity, copy the data and free rhs buffers
	if(capacity() >= rhs_len) {
		memmove(buffer(), rhs.buffer(), rhs_len);
		setlen(rhs_len);
		rhs.invalidate();
		return;
	}

	assert(!rhs.sso.set);

	// We don't have enough space so perform a pointer swap
	if(!sso.set) {
		free(ptr.buffer);
	}
	ptr = rhs.ptr;
	// Can't use rhs.invalidate here as it would free the buffer
	rhs.ptr.buffer = nullptr;
	rhs.ptr.capacity = 0;
	rhs.ptr.len = 0;
}
#endif

String & String::operator = (const String &rhs)
{
  if (this == &rhs) return *this;

  if (rhs.isNull()) {
	  invalidate();
  } else {
	  copy(rhs.cbuffer(), rhs.length());
  }

  return *this;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
String & String::operator = (StringSumHelper && rval)
{
  if (this != &rval) move(rval);
  return *this;
}
#endif

String & String::operator = (const char *cstr)
{
  if (cstr) copy(cstr, strlen(cstr));
  else invalidate();

  return *this;
}

/*********************************************/
/*  concat                                   */
/*********************************************/

bool String::concat(const char *cstr, size_t length)
{
  if (length == 0) return true; // Nothing to add
  if (!cstr) return false; // Bad argument (length is non-zero)

  auto len = this->length();
  size_t newlen = len + length;

  // Appending all or part of self requires special handling
  auto buf = buffer();
  if(cstr >= buf && cstr < (buf + len)) {
	  auto offset = cstr - buf;
	  if (!reserve(newlen)) return false;
	  buf = buffer();
	  memcpy(buf + len, buf + offset, length);
	  setlen(newlen);
	  return true;
  }

  if (!reserve(newlen)) return false;
  memcpy(buffer() + len, cstr, length);
  setlen(newlen);
  return true;
}

bool String::concat(const char *cstr)
{
  if (!cstr) return true; // Consider this an empty string, not a failure
  return concat(cstr, strlen(cstr));
}

bool String::concat(unsigned char num)
{
  char buf[1 + 3 * sizeof(num)];
  itoa(num, buf, 10);
  return concat(buf, strlen(buf));
}

bool String::concat(int num)
{
  char buf[8 + 3 * sizeof(num)];
  itoa(num, buf, 10);
  return concat(buf, strlen(buf));
}

bool String::concat(unsigned int num)
{
  char buf[8 + 3 * sizeof(num)];
  ultoa(num, buf, 10);
  return concat(buf, strlen(buf));
}

bool String::concat(long num)
{
  char buf[8 + 3 * sizeof(num)];
  ltoa(num, buf, 10);
  return concat(buf, strlen(buf));
}

bool String::concat(long long num)
{
  char buf[8 + 3 * sizeof(num)];
  lltoa(num, buf, 10);
  return concat(buf, strlen(buf));
}

bool String::concat(unsigned long num)
{
  char buf[8 + 3 * sizeof(num)];
  ultoa(num, buf, 10);
  return concat(buf, strlen(buf));
}

bool String::concat(unsigned long long num)
{
  char buf[8 + 3 * sizeof(num)];
  ulltoa(num, buf, 10);
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

StringSumHelper & operator + (const StringSumHelper &lhs, const String &rhs)
{
  StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
  if (!a.concat(rhs)) a.invalidate();
  return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, const char *cstr)
{
  StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
  if (!cstr || !a.concat(cstr, strlen(cstr))) a.invalidate();
  return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, char c)
{
  StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
  if (!a.concat(c)) a.invalidate();
  return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, unsigned char num)
{
  StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
  if (!a.concat(num)) a.invalidate();
  return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, int num)
{
  StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
  if (!a.concat(num)) a.invalidate();
  return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, unsigned int num)
{
  StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
  if (!a.concat(num)) a.invalidate();
  return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, long num)
{
  StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
  if (!a.concat(num)) a.invalidate();
  return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, unsigned long num)
{
  StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
  if (!a.concat(num)) a.invalidate();
  return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, float num)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num)) a.invalidate();
	return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, double num)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num)) a.invalidate();
	return a;
}

/*********************************************/
/*  Comparison                               */
/*********************************************/

int String::compareTo(const char* cstr, size_t length) const
{
  auto len = this->length();
  if (len == 0 || length == 0) {
	return len - length;
  }
  auto buf = cbuffer();
  assert(buf != nullptr && cstr != nullptr);
  if(len == length) {
  	return memcmp(buf, cstr, len);
  }
  if(len < length) {
	return memcmp(buf, cstr, len) ?: -1;
  }
  return memcmp(buf, cstr, length) ?: 1;
}

bool String::equals(const char *cstr) const
{
  auto len = length();
  if (len == 0) return (cstr == nullptr || *cstr == '\0');
  if (cstr == nullptr) return false;
  auto cstrlen = strlen(cstr);
  if (len != cstrlen) return false;
  return memcmp(cbuffer(), cstr, len) == 0;
}

bool String::equals(const char *cstr, size_t length) const
{
  auto len = this->length();
  if (len != length) return false;
  if (len == 0) return true;
  return memcmp(cbuffer(), cstr, len) == 0;
}

bool String::equals(const FlashString& fstr) const
{
	auto len = length();
	if (len != fstr.length()) return false;
	LOAD_FSTR(buf, fstr);
	return memcmp(buf, cbuffer(), len) == 0;
}

bool String::equalsIgnoreCase(const char* cstr) const
{
  auto buf = cbuffer();
  if(buf == cstr) return true;
  return strcasecmp(cstr, buf) == 0;
}

bool String::equalsIgnoreCase(const char* cstr, size_t length) const
{
  auto len = this->length();
  if (len != length) return false;
  if (len == 0) return true;
  return memicmp(cbuffer(), cstr, len) == 0;
}

bool String::equalsIgnoreCase(const FlashString& fstr) const
{
  auto len = length();
  if (len != fstr.length()) return false;
  LOAD_FSTR(buf, fstr);
  return memicmp(buf, cbuffer(), len) == 0;
}

bool String::startsWith(const String &prefix) const
{
  if (length() < prefix.length()) return false;
  return startsWith(prefix, 0);
}

bool String::startsWith(const String &prefix, size_t offset) const
{
  auto prefix_buffer = prefix.cbuffer();
  auto prefix_len = prefix.length();
  if (offset + prefix_len > length() || !prefix_buffer) return false;
  return memcmp(&cbuffer()[offset], prefix_buffer, prefix_len) == 0;
}

bool String::endsWith(const String &suffix) const
{
  auto len = length();
  auto suffix_buffer = suffix.cbuffer();
  auto suffix_len = suffix.length();
  if (len < suffix_len || !suffix_buffer) return false;
  return memcmp(&cbuffer()[len - suffix_len], suffix_buffer, suffix_len) == 0;
}

/*********************************************/
/*  Character Access                         */
/*********************************************/

void String::setCharAt(size_t index, char c)
{
  if (index < length()) buffer()[index] = c;
}

char & String::operator[](size_t index)
{
  if (index >= length())
  {
    static char dummy_writable_char;
    dummy_writable_char = '\0';
    return dummy_writable_char;
  }
  return buffer()[index];
}

char String::operator[](size_t index) const
{
  if (index >= length()) return '\0';
  return cbuffer()[index];
}

size_t String::getBytes(unsigned char *buf, size_t bufsize, size_t index) const
{
  if (!bufsize || !buf) return 0;
  auto len = length();
  if (index >= len)
  {
    buf[0] = '\0';
    return 0;
  }
  size_t n = bufsize - 1;
  if (n > len - index) n = len - index;
  memmove(buf, cbuffer() + index, n);
  buf[n] = '\0';
  return n;
}

/*********************************************/
/*  Search                                   */
/*********************************************/

int String::indexOf(char ch, size_t fromIndex) const
{
  auto len = length();
  if (fromIndex >= len) return -1;
  auto buf = cbuffer();
  auto temp = memchr(buf + fromIndex, ch, len - fromIndex);
  if (temp == nullptr) return -1;
  return static_cast<const char*>(temp) - buf;
}

int String::indexOf(const String &s2, size_t fromIndex) const
{
  auto len = length();
  if (fromIndex >= len) return -1;
  auto buf = cbuffer();
  auto found = memmem(buf + fromIndex, len - fromIndex, s2.cbuffer(), s2.length());
  if (found == nullptr) return -1;
  return static_cast<const char*>(found) - buf;
}

int String::lastIndexOf(char theChar) const
{
	auto len = length();
	if(len == 0) {
		return -1;
	}
	auto buf = cbuffer();
	auto found = memrchr(buf, theChar, len);
	return found ? (static_cast<const char*>(found) - buf) : -1;
}

int String::lastIndexOf(char ch, size_t fromIndex) const
{
	auto len = length();
	if(len == 0) {
		return -1;
	}
	if(fromIndex < len) {
		len = fromIndex + 1;
	}
	auto buf = cbuffer();
	auto found = memrchr(buf, ch, len);
	return found ? (static_cast<const char*>(found) - buf) : -1;
}

int String::lastIndexOf(const String &s2) const
{
  return lastIndexOf(s2.cbuffer(), length() - s2.length(), s2.length());
}

int String::lastIndexOf(const String &s2, size_t fromIndex) const
{
	return lastIndexOf(s2.cbuffer(), fromIndex, s2.length());
}

int String::lastIndexOf(const char* s2_buf, size_t fromIndex, size_t s2_len) const
{
  auto len = length();
  if (s2_len == 0 || len == 0 || s2_len > len) return -1;
  auto buf = cbuffer();
  if (fromIndex >= len) fromIndex = len - 1;
  int found = -1;
  for (auto p = buf; p <= buf + fromIndex; p++)
  {
    p = static_cast<const char*>(memmem(p, buf + len - p, s2_buf, s2_len));
    if (!p) break;
    if (p <= buf + fromIndex) found = p - buf;
  }
  return found;
}

String String::substring(size_t left, size_t right) const
{
  if (isNull()) return nullptr;

  if (left > right)
  {
    size_t temp = right;
    right = left;
    left = temp;
  }
  auto len = length();
  if (left >= len) return nullptr;
  if (right > len) right = len;
  return String(cbuffer() + left, right - left);
}

/*********************************************/
/*  Modification                             */
/*********************************************/

void String::replace(char find, char replace)
{
  if (isNull()) return;
  auto len = length();
  auto buf = buffer();
  for (unsigned i = 0; i < len; ++i)
  {
    if (buf[i] == find) buf[i] = replace;
  }
}

void String::replace(const String& find, const String& replace)
{
  auto len = length();
  auto find_len = find.length();
  auto replace_len = replace.length();
  auto buf = buffer();
  auto find_buf = find.cbuffer();
  auto replace_buf = replace.cbuffer();
  if (len == 0 || find_len == 0) return;
  int diff = replace_len - find_len;
  char *readFrom = buf;
  const char* end = buf + len;
  char *foundAt;
  if (diff == 0)
  {
    while ((foundAt = (char*)memmem(readFrom, end - readFrom, find_buf, find_len)) != nullptr)
    {
      memcpy(foundAt, replace_buf, replace_len);
      readFrom = foundAt + replace_len;
    }
  }
  else if (diff < 0)
  {
    char *writeTo = buf;
    while ((foundAt = (char*)memmem(readFrom, end - readFrom, find_buf, find_len)) != nullptr)
    {
      size_t n = foundAt - readFrom;
      memcpy(writeTo, readFrom, n);
      writeTo += n;
      memcpy(writeTo, replace_buf, replace_len);
      writeTo += replace_len;
      readFrom = foundAt + find_len;
      len += diff;
    }
    memcpy(writeTo, readFrom, end - readFrom);
    setlen(len);
  }
  else
  {
    size_t size = len; // compute size needed for result
    while ((foundAt = (char*)memmem(readFrom, end - readFrom, find_buf, find_len)) != nullptr)
    {
      readFrom = foundAt + find_len;
      size += diff;
    }
    if (size == len) return;
    if(!reserve(size)) {
    	return;
    }
    buf = buffer();
    int index = len - 1;
    while ((index = lastIndexOf(find, index)) >= 0)
    {
      readFrom = buf + index + find_len;
      memmove(readFrom + diff, readFrom, len - (readFrom - buf));
      len += diff;
      memcpy(buf + index, replace_buf, replace_len);
      index--;
    }
    setlen(len);
  }
}

void String::remove(size_t index, size_t count)
{
	if (count == 0) { return; }
	auto len = length();
	if (index >= len) { return; }
	if (count > len - index) { count = len - index; }
	char *writeTo = buffer() + index;
	len -= count;
	memcpy(writeTo, writeTo + count, len - index);
	setlen(len);
}

void String::toLowerCase(void)
{
	auto buf = buffer();
	for(unsigned len = length(); len > 0; --len, ++buf) {
		*buf = tolower(*buf);
	}
}

void String::toUpperCase(void)
{
	auto buf = buffer();
	for(unsigned len = length(); len > 0; --len, ++buf) {
		*buf = toupper(*buf);
	}
}

void String::trim(void)
{
  auto len = length();
  if (len == 0) return;
  auto buf = buffer();
  char *begin = buf;
  while (isspace(*begin)) begin++;
  char *end = buf + len - 1;
  while (isspace(*end) && end >= begin) end--;
  len = end + 1 - begin;
  if (begin > buf) memmove(buf, begin, len);
  setlen(len);
}

/*********************************************/
/*  Parsing / Conversion                     */
/*********************************************/

long String::toInt(void) const
{
	return isNull() ? 0 : atoi(cbuffer());
}

float String::toFloat(void) const
{
	return isNull() ? 0.0 : atof(cbuffer());
}
