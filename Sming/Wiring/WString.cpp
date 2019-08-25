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

const String String::nullstr = nullptr;
const String String::empty = "";

/*********************************************/
/*  Constructors                             */
/*********************************************/

String::String(const char *cstr)
{
  if (cstr) copy(cstr, strlen(cstr));
}

String::String(const char *cstr, unsigned int length)
{
  if (cstr) copy(cstr, length);
}

String::String(const String &value)
{
  *this = value;
}

String::String(flash_string_t pstr, int length)
{
  setString(pstr, length);
}

String::String(const FlashString& fstr)
{
  setString(fstr.data(), fstr.length());
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
String::String(String &&rval)
{
  move(rval);
}
String::String(StringSumHelper &&rval)
{
  move(rval);
}
#endif

String::String(char c)
{
  if (setLength(1))
	  buffer[0] = c;
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

String::~String()
{
	free(buffer);
}

void String::setString(const char *cstr, int length /* = -1 */)
{
	if (cstr)
	{
		if (length < 0)
			length = strlen(cstr);
		copy(cstr, length);
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
}
/*********************************************/
/*  Memory Management                        */
/*********************************************/

void String::invalidate(void)
{
  if (buffer) free(buffer);
  buffer = nullptr;
  capacity = len = 0;
}

bool String::reserve(unsigned int size)
{
  if (buffer && capacity >= size) return true;
  if (changeBuffer(size))
  {
    if (len == 0) buffer[0] = '\0';
    return true;
  }
  return false;
}

bool String::setLength(unsigned int size)
{
	if(!reserve(size))
		return false;

	len = size;
	if(buffer)
		buffer[len] = '\0';

	return true;
}

bool String::changeBuffer(unsigned int maxStrLen)
{
  char *newbuffer = (char *)realloc(buffer, maxStrLen + 1);
  if (newbuffer)
  {
    buffer = newbuffer;
    capacity = maxStrLen;
    return true;
  }
  return false;
}

/*********************************************/
/*  Copy and Move                            */
/*********************************************/

String & String::copy(const char *cstr, unsigned int length)
{
  if (!reserve(length))
  {
    invalidate();
    return *this;
  }
  len = length;
  memmove(buffer, cstr, length);
  buffer[length] = '\0';
  return *this;
}

String &String::copy(flash_string_t pstr, unsigned int length)
{
	// If necessary, allocate additional space so copy can be aligned
	unsigned int length_aligned = ALIGNUP(length);
	if(!reserve(length_aligned))
	{
		invalidate();
	}
	else
	{
		memcpy_aligned(buffer, (PGM_P)pstr, length);
		buffer[length] = '\0';
		len = length;
	}
	return *this;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void String::move(String &rhs)
{
  if (buffer)
	  free(buffer);
  buffer = rhs.buffer;
  capacity = rhs.capacity;
  len = rhs.len;
  rhs.buffer = nullptr;
  rhs.capacity = 0;
  rhs.len = 0;
}
#endif

String & String::operator = (const String &rhs)
{
  if (this == &rhs) return *this;

  if (rhs.buffer) copy(rhs.buffer, rhs.len);
  else invalidate();

  return *this;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
String & String::operator = (String && rval)
{
  if (this != &rval) move(rval);
  return *this;
}

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

bool String::concat(const String &s)
{
  return concat(s.buffer, s.len);
}

bool String::concat(const char *cstr, unsigned int length)
{
  unsigned int newlen = len + length;
  if (length == 0) return true; // Nothing to add
  if (!cstr) return false; // Bad argument (length is non-zero)
  if (!reserve(newlen)) return false;
  memmove(buffer + len, cstr, length);
  buffer[newlen] = '\0';
  len = newlen;
  return true;
}

bool String::concat(const char *cstr)
{
  if (!cstr) return true; // Consider this an empty string, not a failure
  return concat(cstr, strlen(cstr));
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
  if (!a.concat(rhs.buffer, rhs.len)) a.invalidate();
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

int String::compareTo(const String &s) const
{
  if (!buffer || !s.buffer)
  {
    if (s.buffer && s.len > 0) return 0 - *(unsigned char *)s.buffer;
    if (buffer && len > 0) return *(unsigned char *)buffer;
    return 0;
  }
  return strcmp(buffer, s.buffer);
}

bool String::equals(const String &s2) const
{
  return (len == s2.len && memcmp(buffer, s2.buffer, len) == 0);
}

bool String::equals(const char *cstr) const
{
  if (len == 0) return (cstr == nullptr || *cstr == '\0');
  if (cstr == nullptr) return buffer[0] == '\0';
  // Don't use strcmp as data may contain nuls
  size_t cstrlen = strlen(cstr);
  if (len != cstrlen) return false;
  return memcmp(buffer, cstr, len) == 0;
}

bool String::equals(const FlashString& fstr) const
{
	if (len != fstr.length()) return false;
	LOAD_FSTR(buf, fstr);
	return memcmp(buf, buffer, len) == 0;
}

bool String::operator<(const String &rhs) const
{
  return compareTo(rhs) < 0;
}

bool String::operator>(const String &rhs) const
{
  return compareTo(rhs) > 0;
}

bool String::operator<=(const String &rhs) const
{
  return compareTo(rhs) <= 0;
}

bool String::operator>=(const String &rhs) const
{
  return compareTo(rhs) >= 0;
}

bool String::equalsIgnoreCase(const char* cstr) const
{
  if(buffer == cstr) return true;
  return strcasecmp(cstr, buffer) == 0;
}

bool String::equalsIgnoreCase(const String &s2) const
{
  if (len != s2.len) return false;
  if (len == 0) return true;
  return equalsIgnoreCase(s2.buffer);
}

bool String::equalsIgnoreCase(const FlashString& fstr) const
{
  if (len != fstr.length()) return false;
  LOAD_FSTR(buf, fstr);
  return strcasecmp(buf, buffer) == 0;
}

bool String::startsWith(const String &prefix) const
{
  if (len < prefix.len) return false;
  return startsWith(prefix, 0);
}

bool String::startsWith(const String &prefix, unsigned int offset) const
{
  if (offset + prefix.len > len || !buffer || !prefix.buffer) return false;
  return memcmp(&buffer[offset], prefix.buffer, prefix.len) == 0;
}

bool String::endsWith(const String &suffix) const
{
  if (len < suffix.len || !buffer || !suffix.buffer) return false;
  return memcmp(&buffer[len - suffix.len], suffix.buffer, suffix.len) == 0;
}

/*********************************************/
/*  Character Access                         */
/*********************************************/

char String::charAt(unsigned int index) const
{
  return operator[](index);
}

void String::setCharAt(unsigned int index, char c)
{
  if (index < len) buffer[index] = c;
}

char & String::operator[](unsigned int index)
{
  if (index >= len || !buffer)
  {
    static char dummy_writable_char;
    dummy_writable_char = '\0';
    return dummy_writable_char;
  }
  return buffer[index];
}

char String::operator[](unsigned int index) const
{
  if (index >= len || !buffer) return '\0';
  return buffer[index];
}

unsigned int String::getBytes(unsigned char *buf, unsigned int bufsize, unsigned int index) const
{
  if (!bufsize || !buf) return 0;
  if (index >= len)
  {
    buf[0] = '\0';
    return 0;
  }
  unsigned int n = bufsize - 1;
  if (n > len - index) n = len - index;
  memmove(buf, buffer + index, n);
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

int String::indexOf(char ch, unsigned int fromIndex) const
{
  if (fromIndex >= len) return -1;
  auto temp = (const char*)memchr(buffer + fromIndex, ch, len - fromIndex);
  if (temp == nullptr) return -1;
  return temp - buffer;
}

int String::indexOf(const String &s2) const
{
  return indexOf(s2, 0);
}

int String::indexOf(const String &s2, unsigned int fromIndex) const
{
  if (fromIndex >= len) return -1;
  auto found = (const char*)memmem(buffer + fromIndex, len - fromIndex, s2.buffer, s2.len);
  if (found == nullptr) return -1;
  return found - buffer;
}

int String::lastIndexOf(char theChar) const
{
  return lastIndexOf(theChar, len - 1);
}

int String::lastIndexOf(char ch, unsigned int fromIndex) const
{
  if (fromIndex >= len) return -1;
  char tempchar = buffer[fromIndex + 1];
  buffer[fromIndex + 1] = '\0';
  char* temp = strrchr(buffer, ch);
  buffer[fromIndex + 1] = tempchar;
  if (temp == nullptr) return -1;
  return temp - buffer;
}

int String::lastIndexOf(const String &s2) const
{
  return lastIndexOf(s2, len - s2.len);
}

int String::lastIndexOf(const String &s2, unsigned int fromIndex) const
{
  if (s2.len == 0 || len == 0 || s2.len > len) return -1;
  if (fromIndex >= len) fromIndex = len - 1;
  int found = -1;
  for (char *p = buffer; p <= buffer + fromIndex; p++)
  {
    p = (char*)memmem(p, buffer + len - p, s2.buffer, s2.len);
    if (!p) break;
    if (p <= buffer + fromIndex) found = p - buffer;
  }
  return found;
}

String String::substring(unsigned int left, unsigned int right) const
{
  if (!buffer) return nullptr;

  if (left > right)
  {
    unsigned int temp = right;
    right = left;
    left = temp;
  }
  String out;
  if (left > len) return out;
  if (right > len) right = len;
  char temp = buffer[right];  // save the replaced character
  buffer[right] = '\0';
  out = buffer + left;  // pointer arithmetic
  buffer[right] = temp;  //restore character
  return out;
}

/*********************************************/
/*  Modification                             */
/*********************************************/

void String::replace(char find, char replace)
{
  if (!buffer) return;
  for (char *p = buffer; *p; p++)
  {
    if (*p == find) *p = replace;
  }
}

void String::replace(const String& find, const String& replace)
{
  if (len == 0 || find.len == 0) return;
  int diff = replace.len - find.len;
  char *readFrom = buffer;
  const char* end = buffer + len;
  char *foundAt;
  if (diff == 0)
  {
    while ((foundAt = (char*)memmem(readFrom, end - readFrom, find.buffer, find.len)) != nullptr)
    {
      memcpy(foundAt, replace.buffer, replace.len);
      readFrom = foundAt + replace.len;
    }
  }
  else if (diff < 0)
  {
    char *writeTo = buffer;
    while ((foundAt = (char*)memmem(readFrom, end - readFrom, find.buffer, find.len)) != nullptr)
    {
      unsigned int n = foundAt - readFrom;
      memcpy(writeTo, readFrom, n);
      writeTo += n;
      memcpy(writeTo, replace.buffer, replace.len);
      writeTo += replace.len;
      readFrom = foundAt + find.len;
      len += diff;
    }
    memcpy(writeTo, readFrom, end - readFrom);
    buffer[len] = '\0';
  }
  else
  {
    unsigned int size = len; // compute size needed for result
    while ((foundAt = (char*)memmem(readFrom, end - readFrom, find.buffer, find.len)) != nullptr)
    {
      readFrom = foundAt + find.len;
      size += diff;
    }
    if (size == len) return;
    if (size > capacity && !changeBuffer(size)) return; // XXX: tell user!
    int index = len - 1;
    while ((index = lastIndexOf(find, index)) >= 0)
    {
      readFrom = buffer + index + find.len;
      memmove(readFrom + diff, readFrom, len - (readFrom - buffer));
      len += diff;
      memcpy(buffer + index, replace.buffer, replace.len);
      index--;
    }
    buffer[len] = '\0';
  }
}

void String::remove(unsigned int index)
{
	if(index < len) remove(index, len - index);
}

void String::remove(unsigned int index, unsigned int count)
{
	if (index >= len) { return; }
	if (count == 0) { return; }
	if (index + count > len) { count = len - index; }
	char *writeTo = buffer + index;
	len -= count;
	memcpy(writeTo, writeTo + count, len - index);
	buffer[len] = '\0';
}

void String::toLowerCase(void)
{
  if (!buffer) return;
  for (char *p = buffer; *p; p++)
  {
    *p = tolower(*p);
  }
}

void String::toUpperCase(void)
{
  if (!buffer) return;
  for (char *p = buffer; *p; p++)
  {
    *p = toupper(*p);
  }
}

void String::trim(void)
{
  if (!buffer || len == 0) return;
  char *begin = buffer;
  while (isspace(*begin)) begin++;
  char *end = buffer + len - 1;
  while (isspace(*end) && end >= begin) end--;
  len = end + 1 - begin;
  if (begin > buffer) memmove(buffer, begin, len);
  buffer[len] = '\0';
}

/*********************************************/
/*  Parsing / Conversion                     */
/*********************************************/

long String::toInt(void) const
{
  if (buffer) return atoi(buffer);
  return 0;
}

float String::toFloat(void) const
{
  if (buffer) return (float)atof(buffer);
  return 0;
}

/*void String::printTo(Print &p) const
{
  p.print(buffer);
}*/

