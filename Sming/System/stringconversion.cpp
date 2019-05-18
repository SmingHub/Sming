/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StringConversion.cpp
 *
 ****/

#include <user_config.h>
#include <math.h>
#include <stdlib.h>
#include "stringconversion.h"
#include "stringutil.h"

//Since C does not support default func parameters, keep this function as used by framework
//and create extended _w funct to handle width
char* ltoa(long val, char* buffer, int base)
{
	return ltoa_w(val, buffer, base, 0);
}

char* itoa (int, char*, int) __attribute__((alias("ltoa")));

char* ltoa_w(long val, char* buffer, int base, int width)
{
	return ltoa_wp(val, buffer, base, width, ' ');
}

char* ltoa_wp(long val, char* buffer, int base, int width, char pad)
{
	int i = 38, p = 0;
	char buf[40] = {0};
	bool ngt = val < 0;
	if (ngt) val = -val;

	for(; val && i ; --i, p++, val /= base)
		buf[i] = hexchar(val % base);
	if (p == 0) buf[i--] = '0'; // case for zero

	if (ngt)
		buf[i--] = '-';

	if(width != 0)
	{
		width -= strlen(&buf[i+1]);
		if(width > 0)
		{
			memset(buffer, pad, width);
		}
		else width = 0;
	}

	strcpy(buffer + width, &buf[i+1]);
	return buffer;
}

//Since C does not support default func parameters, keep this function as used by framework
//and create extended _w funct to handle width
char* ultoa(unsigned long val, char* buffer, unsigned int base)
{
	return ultoa_w(val, buffer, base, 0);
}

char* ultoa_w(unsigned long val, char* buffer, unsigned int base, int width)
{
	return ultoa_wp(val, buffer, base, 0, ' ');
}
char* ultoa_wp(unsigned long val, char* buffer, unsigned int base, int width, char pad)
{
	int i = 38, p = 0;
	char buf[40] = {0};

	for(; val && i ; --i, p++, val /= base)
		buf[i] = hexchar(val % base);
	if (p == 0) buf[i--] = '0'; // case for zero

	if(width != 0)
	{
		width -= strlen(&buf[i+1]);
		if(width > 0)
		{
			memset(buffer, pad, width);
		}
		else width = 0;
	}
	strcpy(buffer + width, &buf[i+1]);

	return buffer;
}

char *dtostrf(double floatVar, int minStringWidthIncDecimalPoint, int numDigitsAfterDecimal, char *outputBuffer)
{
	return dtostrf_p(floatVar, minStringWidthIncDecimalPoint, numDigitsAfterDecimal, outputBuffer, ' ');
}
// Author zitron: http://forum.arduino.cc/index.php?topic=37391#msg276209
// modified by ADiea: remove dependencies strcat, floor, round; reorganize+speedup code
char *dtostrf_p(double floatVar, int minStringWidthIncDecimalPoint, int numDigitsAfterDecimal, char *outputBuffer, char pad)
{
	char temp[40], num[40];
	unsigned long mult = 1, int_part;
	int16_t i, processedFracLen = numDigitsAfterDecimal;

	if(processedFracLen < 0)
		processedFracLen = 9;

	if (outputBuffer == nullptr)
		return nullptr;

	if (isnan(floatVar))
		strcpy(outputBuffer, "NaN");
	else if (isinf(floatVar))
		strcpy(outputBuffer, "Inf");
	else if (floatVar > 4294967040.0)  // constant determined empirically
		strcpy(outputBuffer, "OVF");
	else if (floatVar < -4294967040.0)   // constant determined empirically
		strcpy(outputBuffer, "ovf");
	else
	{
		//start building the number
		//buf will be the end pointer
		char* buf = num;

		if (floatVar < 0.0)
		{
			*buf++ = '-';  //print "-" sign
			floatVar = -floatVar;
		}

		// Extract the integer part of the number and print it

		if (processedFracLen > 9)
			processedFracLen = 9; // Prevent overflow!

		i = processedFracLen;

		while (i-- > 0)
			mult *= 10;

		//round the number
		floatVar += 0.5 / (float) mult;

		int_part = (unsigned long) floatVar;

		//print the int part into num
		char* s = ltoa(int_part, buf, 10);

		//adjust end pointer
		buf += strlen(s); //go to end of string

		//deal with digits after the decimal
		if (numDigitsAfterDecimal != 0)
		{
			*buf++ = '.'; // print the decimal point

			//print the fraction part into temp
			s = ltoa( ((floatVar - int_part) * mult), temp, 10);

			i = processedFracLen - strlen(s) + 1;

			//print the first zeros of the fraction part
			while (--i > 0)
				*buf++ = '0';

			//print the fraction part
			while (*s)
				*buf++ = *s++;

			//trim back on the last fraction zeroes
			while(*(buf - 1) == '0')
			{
				--buf;
				--processedFracLen;
			}

			if(numDigitsAfterDecimal > 0)
			{
				i = numDigitsAfterDecimal - processedFracLen;

				// padding fraction zeroes
				while (i-- > 0)
					*buf++ = '0';
			}
		}

		//terminate num
		*buf = 0;

		//switch buf to outputBuffer
		buf = outputBuffer;

		// generate width space padding
		i = minStringWidthIncDecimalPoint - strlen(num) + 1;
		while (--i > 0)
		{
			*buf++ = pad;
		}

		//Write output buffer
		s = num;
		while (*s)
			*buf++ = *s++;

		//termninate outputBuffer
		*buf = 0;
	}

	return outputBuffer;
}
