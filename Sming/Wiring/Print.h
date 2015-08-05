/* $Id: Print.h 1156 2011-06-07 04:01:16Z bhagman $
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

#ifndef PRINT_H
#define PRINT_H

#ifdef __cplusplus
#include "WiringFrameworkDependencies.h"
#include "Printable.h"
#include "WString.h"

#define INITIAL_PRINTF_BUFFSIZE 128

class String;

class Print
{
  public:
	Print() : write_error(0) {}
  
    int getWriteError() { return write_error; }
    void clearWriteError() { setWriteError(0); }
  
    // pure virtual - must be implemented by derived class
    //virtual void write(uint8_t) = 0;

    // virtual - can be redefined (polymorphic class)
    //virtual void write(const char *str);
    //virtual void write(const uint8_t *buffer, size_t size);

    virtual size_t write(uint8_t) = 0;
    size_t write(const char *str) {
      if (str == NULL) return 0;
      return write((const uint8_t *)str, strlen(str));
    }
    virtual size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *buffer, size_t size) {
      return write((const uint8_t *)buffer, size);
    }
  
    // print
    size_t print(const __FlashStringHelper *);
    size_t print(char);
    size_t print(const char[]);

    size_t print(unsigned long, int base = DEC);
    size_t print(long, int base = DEC);

    size_t print(unsigned int, int base = DEC);
    size_t print(unsigned char, int base = DEC);
    size_t print(int, int base = DEC);
    size_t print(double, int digits = 2);

    size_t print(const Printable &p);
    size_t print(const String &s);
  
    // println
    size_t println(const __FlashStringHelper *);
    size_t println(void);

    size_t println(const char[]);
    size_t println(char);

    size_t println(unsigned char, int base = DEC);
    size_t println(unsigned int, int base = DEC);
    size_t println(unsigned long, int base = DEC);
    size_t println(int, int base = DEC);
    size_t println(long, int base = DEC);
    size_t println(double, int digits = 2);

    size_t println(const Printable &p);
    size_t println(const String &s);

    // printf
    size_t printf(const char *fmt, ...);

  private:
    int write_error;
    size_t printNumber(unsigned long, uint8_t);
    size_t printFloat(double, uint8_t);
  protected:
    void setWriteError(int err = 1) { write_error = err; }
};

#endif  // __cplusplus
#endif
// PRINT_H
