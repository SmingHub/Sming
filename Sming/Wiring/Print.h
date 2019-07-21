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

#pragma once

#ifdef __cplusplus

#include "WiringFrameworkDependencies.h"
#include "Printable.h"

#define INITIAL_PRINTF_BUFFSIZE 128

class String;

/** @defgroup print Print stream functions
    @brief  Provides functions that format output to streams, e.g. Serial
    @{
*/

/** @brief  Provides formatted output to stream
 */
class Print
{
  public:
	Print() {}
	virtual ~Print() {}
  
    /** @brief  Gets last error
    	@retval int Error number of last write error
    */
    int getWriteError() { return write_error; }

    /** @brief  Clears the last write error
     */
    void clearWriteError() { setWriteError(0); }
  
    // pure virtual - must be implemented by derived class
    //virtual void write(uint8_t) = 0;

    // virtual - can be redefined (polymorphic class)
    //virtual void write(const char *str);
    //virtual void write(const uint8_t *buffer, size_t size);

	/** @brief  Writes a single character to output stream
	  * @param  uint8_t Character to write to output stream
	  * @retval size_t Quantity of characters written to output stream
	  */
    virtual size_t write(uint8_t) = 0;

    /**	@brief  Writes a c-string to output stream
	  * @param  str Pointer to c-string
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t write(const char *str) {
      if (str == NULL) return 0;
      return write((const uint8_t *)str, strlen(str));
    }

    /**	@brief  Writes characters from a buffer to output stream
	  * @param  buffer Pointer to character buffer
	  * @param  size Quantity of characters to write
	  * @retval size_t Quantity of characters written to stream
	  */
    virtual size_t write(const uint8_t *buffer, size_t size);

    /** @brief  Writes characters from a buffer to output stream
	  * @param  buffer Pointer to character buffer
	  * @param  size Quantity of characters to write
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t write(const char *buffer, size_t size) {
      return write((const uint8_t *)buffer, size);
    }
  
    /** @brief  Prints a single character to output stream
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t print(char);

    /** @brief  Prints a c-string to output stream
	  * @param  char[] c-string to print
	  * @retval size_t Quantity of characters written to output stream
	  */
	size_t print(const char[]);

    /** @brief  Prints a number to output stream
	  * @param  "unsigned long" Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t print(unsigned long, int base = DEC);

    /** @brief  Prints a number to output stream
	  * @param  "long" Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t print(long, int base = DEC);

    /** @brief  Prints a number to output stream
	  * @param  "unsigned int" Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t print(unsigned int, int base = DEC);

    /** @brief  Prints a number to output stream
	  * @param  "unsigned char" Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t print(unsigned char, int base = DEC);

    /** @brief  Prints a number to output stream
	  * @param  "int" Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t print(int, int base = DEC);

    /** @brief  Prints a number to output stream
	  * @param  "double" Number to print
	  * @param  digits The decimal places to print (Default: 2, e.g. 21.35)
	  * @retval size_t Quantity of characters written to stream
	  */
	size_t print(double, int digits = 2);

    /** @brief  Prints a Printable object to output stream
	  * @param  p Object to print
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t print(const Printable &p);

    /** @brief  Prints a String to output stream
	  * @param  s String to print
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t print(const String &s);
  
    /** @brief  Prints a newline to output stream
	  * @retval size_t Quantity of characters written to stream
	  */
	size_t println(void);

    /** @brief  Prints a c-string to output stream, appending newline
	  * @param  "char[]" c-string to print
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t println(const char[]);

    /** @brief  Prints a single character to output stream, appending newline
	  * @param  "char" Character to print
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t println(char);

    /** @brief  Prints a number to output stream, appending newline
	  * @param  "unsigned char" Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t println(unsigned char, int base = DEC);

    /** @brief  Prints a number to output stream, appending newline
	  * @param  "unsigned int" Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t println(unsigned int, int base = DEC);

    /** @brief  Prints a number to output stream, appending newline
	  * @param  "unsigned long" Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t println(unsigned long, int base = DEC);

    /** @brief  Prints a number to output stream, appending newline
	  * @param  "int" Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t println(int, int base = DEC);

    /** @brief  Prints a number to output stream, appending newline
	  * @param  "long" Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  */
	size_t println(long, int base = DEC);

    /** @brief  Prints a number to output stream, appending newline
	  * @param  "double" Number to print
	  * @param  digits The decimal places to print (Default: 2, e.g. 21.35)
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t println(double, int digits = 2);

    /** @brief  Prints a Printable object to output stream, appending newline
	  * @param  p Object to print
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t println(const Printable &p);

    /** @brief  Prints a String to output stream, appending newline
	  * @param  s String to print
	  * @retval size_t Quantity of characters written to stream
	  */
    size_t println(const String &s);

    /** @brief  Prints a formatted c-string to output stream
	  * @param  fmt Pointer to formated c-string to print
	  * @param  "..." Parameters for placeholders within formated string
	  * @retval size_t Quantity of characters written to stream
	  * @note   Use standard printf placeholders, e.g. %d for integer, %s for c-string, etc.
	  */
    size_t printf(const char *fmt, ...);

  private:
    int write_error = 0;
    size_t printNumber(unsigned long, uint8_t);
    size_t printFloat(double, uint8_t);
  protected:
    void setWriteError(int err = 1) { write_error = err; }
};

/** @} */

#endif  // __cplusplus
