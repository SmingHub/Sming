/* $Id: Print.cpp 1156 2011-06-07 04:01:16Z bhagman $
||
|| @author         Hernando Barragan <b@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Nicholas Zambetti
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
||
|| @description
|| | Print library.
|| |
|| | Wiring Common API
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#include "Print.h"
#include "WString.h"

/*
|| @description
|| | Virtual method - may be redefined in derived class (polymorphic)
|| | write()s a specific length string.
|| #
*/

size_t Print::write(const uint8_t *buffer, size_t size)
{
  size_t n = 0;
  while (size--) {
    n += write(*buffer++);
  }
  return n;
}


// Base method (signed)
size_t Print::print(long num, int base)
{
  if (base == 0)
  {
    return write(num);
  }

  if (base == 10 && num < 0)
  {
    return print('-') + printNumber((unsigned long)-num, base);
  }

  return printNumber((unsigned long)num, base);
}

// Overload (signed long long)
size_t Print::print(const long long& num, int base)
{
  if (base == 10 && num < 0)
  {
    return print('-') + printNumber((unsigned long long)-num, base);
  }

  return printNumber((unsigned long long)num, base);
}

size_t Print::printf(const char *fmt, ...)
{
	size_t sz = 0;
	size_t buffSize = INITIAL_PRINTF_BUFFSIZE;
	bool retry = false;
	do {
		char tempBuff[buffSize];
		va_list va;
		va_start(va, fmt);
		sz = m_vsnprintf(tempBuff,buffSize, fmt, va);
		va_end(va);
		if (sz > (buffSize -1))
		{
			buffSize = sz + 1; // Leave room for terminating null char
			retry = true;
		}
		else
		{
			if (sz > 0)
			{
				write(tempBuff,sz);
			}
			break;
		}
	} while (retry);
	return sz;
}

// private methods

size_t Print::printNumber(unsigned long num, uint8_t base)
{
  char buf[8 * sizeof(num) + 1]; // Assumes 8-bit chars plus zero byte.
  ultoa(num, buf, base);
  return write(buf);
}

size_t Print::printNumber(const unsigned long long& num, uint8_t base)
{
  char buf[8 * sizeof(num) + 1]; // Assumes 8-bit chars plus zero byte.
  ulltoa(num, buf, base);
  return write(buf);
}

size_t Print::printFloat(double number, uint8_t digits)
{
  size_t n = 0;
  
  if (isnan(number)) return print("nan");
  if (isinf(number)) return print("inf");
  if (number > 4294967040.0) return print ("ovf");  // constant determined empirically
  if (number <-4294967040.0) return print ("ovf");  // constant determined empirically
  
  // Handle negative numbers
  if (number < 0.0)
  {
    n += print('-');
    number = -number;
  }
  
  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i) {
	rounding /= 10.0;
  }
  
  number += rounding;
  
  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  n += print(int_part);
  
  // Print the decimal point, but only if there are digits beyond
  if (digits > 0) {
    n += print(".");
  }
  
  // Extract digits from the remainder one at a time
  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toPrint = int(remainder);
    n += print(toPrint);
    remainder -= toPrint;
  }
  
  return n;
}

