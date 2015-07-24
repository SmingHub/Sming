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
#include "WiringFrameworkIncludes.h"
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


// Base method (character)
size_t Print::print(char c)
{
  return write(c);
}

// Base method (string)
size_t Print::print(const char c[])
{
  return write(c);
}


// Base method (unsigned)
size_t Print::print(unsigned long n, int base)
{
  if (base == 0) return write(n);
  else return printNumber(n, base);
}

// Base method (signed)
size_t Print::print(long n, int base)
{
  if (base == 0)
  {
    return write(n);
  }
  else if (base == 10)
  {
    // why must this only be in base 10?
    if (n < 0)
    {
      int t = print('-');
      n = -n;
      return printNumber(n, 10) + t;
    }
    return printNumber(n, 10);
  }
  else
  {
    return printNumber(n, base);
  }
}


// Overload (unsigned)
size_t Print::print(unsigned int n, int base)
{
  return print((unsigned long)n, base);
}

// Overload (unsigned)
size_t Print::print(unsigned char n, int base)
{
  return print((unsigned long) n, base);
}

// Overload (signed)
size_t Print::print(int n, int base)
{
  return print((long)n, base);
}


size_t Print::print(double n, int digits)
{
  return printFloat(n, digits);
}


size_t Print::print(const Printable &p)
{
  return p.printTo(*this);
}

size_t Print::print(const String &s)
{
  return write(s.c_str(), s.length());
}


size_t Print::println(void)
{
  size_t n = print('\r');
  n += print('\n');
  return n;
}


size_t Print::println(const String &s)
{
  size_t n = print(s);
  n += println();
  return n;
}


size_t Print::println(char c)
{
  size_t n = print(c);
  n += println();
  return n;
}

size_t Print::println(const char c[])
{
  size_t n = print(c);
  n += println();
  return n;
}


size_t Print::println(unsigned long num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(unsigned int num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(unsigned char b, int base)
{
  size_t n = print(b, base);
  n += println();
  return n;
}

size_t Print::println(long num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(int num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(double num, int digits)
{
  size_t n = print(num, digits);
  n += println();
  return n;
}


size_t Print::println(const Printable &p)
{
  size_t n = print(p);
  n += println();
  return n;
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
			return sz;
		}
	} while (retry);
}

// private methods

size_t Print::printNumber(unsigned long n, uint8_t base)
{
  /* BH: new version to be implemented
    uint8_t buf[sizeof(char) * sizeof(int32_t)];
    uint32_t i = 0;

    if (n == 0)
    {
      write('0');
      return;
    }

    while (n > 0)
    {
      buf[i++] = n % base;
      n /= base;
    }

    for (; i > 0; i--)
      write((buf[i - 1] < 10 ?
            '0' + buf[i - 1] :
            'A' + buf[i - 1] - 10));
  */

  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];
  
  *str = '\0';
  
  // prevent crash if called with base == 1
  if (base < 2) base = 10;
  
  do {
    unsigned long m = n;
    n /= base;
    char c = m - base * n;
    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);
  
  return write(str);
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
  for (uint8_t i=0; i<digits; ++i)
  rounding /= 10.0;
  
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

