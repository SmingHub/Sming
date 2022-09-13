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

#include "Printable.h"
#include "WString.h"

#define INITIAL_PRINTF_BUFFSIZE 128

/** @defgroup print Print stream functions
    @brief  Provides functions that format output to streams, e.g. Serial
    @{
*/

/** @brief  Provides formatted output to stream
 *  @ingroup wiring
 */
class Print
{
public:
	virtual ~Print()
	{
	}

	/** @brief  Gets last error
    	@retval int Error number of last write error
    */
	int getWriteError()
	{
		return write_error;
	}

	/** @brief  Clears the last write error
     */
	void clearWriteError()
	{
		setWriteError(0);
	}

	/** @brief  Writes a single character to output stream
	  * @param  c Character to write to output stream
	  * @retval size_t Quantity of characters written to output stream
	  */
	virtual size_t write(uint8_t c) = 0;

	/**	@brief  Writes a c-string to output stream
	  * @param  str Pointer to c-string
	  * @retval size_t Quantity of characters written to stream
	  */
	size_t write(const char* str)
	{
		if(str == nullptr) {
			return 0;
		}
		return write(reinterpret_cast<const uint8_t*>(str), strlen(str));
	}

	/**	@brief  Writes characters from a buffer to output stream
	  * @param  buffer Pointer to character buffer
	  * @param  size Quantity of characters to write
	  * @retval size_t Quantity of characters written to stream
	  */
	virtual size_t write(const uint8_t* buffer, size_t size);

	/** @brief  Writes characters from a buffer to output stream
	  * @param  buffer Pointer to character buffer
	  * @param  size Quantity of characters to write
	  * @retval size_t Quantity of characters written to stream
	  */
	size_t write(const char* buffer, size_t size)
	{
		return write((const uint8_t*)buffer, size);
	}

	/** @brief  Prints a single character to output stream
     *  @param  c Character to print
	  * @retval size_t Quantity of characters written to stream
	  */
	size_t print(char c)
	{
		return write(c);
	}

	/** @brief  Prints a c-string to output stream
	  * @param  str c-string to print
	  * @retval size_t Quantity of characters written to output stream
	  */
	size_t print(const char str[])
	{
		return write(str);
	}

	/** @name   Print an integral number to output stream
	  * @param  num Number to print
	  * @param  base The base for output (Default: Decimal (base 10))
	  * @retval size_t Quantity of characters written to stream
	  *
	  * @{
	  */
	size_t print(unsigned long num, uint8_t base = DEC)
	{
		if(base == 0) {
			return write(num);
		} else {
			return printNumber(num, base);
		}
	}

	template <typename... Args> size_t print(unsigned long num, Args... args)
	{
		return printNumber(num, args...);
	}

	template <typename... Args> size_t print(const unsigned long long& num, Args... args)
	{
		return printNumber(num, args...);
	}

	size_t print(long num, uint8_t base = DEC)
	{
		if(base == 0) {
			return write(num);
		} else {
			return printNumber(num, base);
		}
	}

	template <typename... Args> size_t print(long num, Args... args)
	{
		return printNumber(num, args...);
	}

	template <typename... Args> size_t print(const long long& num, Args... args)
	{
		return printNumber(num, args...);
	}

	template <typename... Args> size_t print(unsigned int num, Args... args)
	{
		return print((unsigned long)num, args...);
	}

	template <typename... Args> size_t print(unsigned char num, Args... args)
	{
		return print((unsigned long)num, args...);
	}

	template <typename... Args> size_t print(int num, Args... args)
	{
		return printNumber((long)num, args...);
	}
	/** @} */

	/** @brief  Print a floating-point number to output stream
	  * @param  num Number to print
	  * @param  digits The decimal places to print (Default: 2, e.g. 21.35)
	  * @retval size_t Quantity of characters written to stream
	  */
	size_t print(double num, int digits = 2)
	{
		return printFloat(num, digits);
	}

	/** @brief  Prints a Printable object to output stream
	  * @param  p Object to print
	  * @retval size_t Quantity of characters written to stream
	  */
	size_t print(const Printable& p)
	{
		return p.printTo(*this);
	}

	/** @brief  Prints a String to output stream
	  * @param  s String to print
	  * @retval size_t Quantity of characters written to stream
	  */
	size_t print(const String& s)
	{
		return write(s.c_str(), s.length());
	}

	/**
	 * @brief enums can be printed as strings provided they have a `toString(E)` implementation.
	 */
	template <typename E>
	typename std::enable_if<std::is_enum<E>::value && !std::is_convertible<E, int>::value, size_t>::type print(E value)
	{
		extern String toString(E e);
		return print(toString(value));
	}

	/** @brief  Prints a newline to output stream
	  * @retval size_t Quantity of characters written to stream
	  */
	size_t println()
	{
		return print("\r\n");
	}

	/** @brief Print value plus newline to output stream
	  * @retval size_t Quantity of characters written to stream
	  */
	template <typename... Args> size_t println(const Args&... args)
	{
		return print(args...) + println();
	}

	/** @brief  Prints a formatted c-string to output stream
	  * @param  fmt Pointer to formatted c-string to print
	  * @param  "..." Parameters for placeholders within formatted string
	  * @retval size_t Quantity of characters written to stream
	  * @note   Use standard printf placeholders, e.g. %d for integer, %s for c-string, etc.
	  */
	size_t printf(const char* fmt, ...) __attribute__((format(printf, 2, 3)));

private:
	int write_error = 0;
	size_t printNumber(unsigned long num, uint8_t base = DEC, uint8_t width = 0, char pad = '0');
	size_t printNumber(const unsigned long long& num, uint8_t base = DEC, uint8_t width = 0, char pad = '0');
	size_t printNumber(long num, uint8_t base = DEC, uint8_t width = 0, char pad = '0');
	size_t printNumber(const long long& num, uint8_t base = DEC, uint8_t width = 0, char pad = '0');
	size_t printFloat(double num, uint8_t digits);

protected:
	void setWriteError(int err = 1)
	{
		write_error = err;
	}
};

template <typename T> Print& operator<<(Print& p, const T& value)
{
	p.print(value);
	return p;
}

// Thanks to Arduino forum user Paul V. who suggested this
// clever technique to allow for expressions like
//   Serial << "Hello!" << endl;
enum EndLineCode { endl };

inline Print& operator<<(Print& p, EndLineCode)
{
	p.println();
	return p;
}

/** @} */

#endif // __cplusplus
