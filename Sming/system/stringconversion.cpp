#include <user_config.h>
#include <math.h>
#include <stdlib.h>
#include "../include/stringconversion.h"

char* ltoa(long val, char* buffer, int base)
{
	int i = 34, p = 0;
	char buf[36] = {0};
	bool ngt = val < 0;
	if (ngt) val = -val;

	for(; val && i ; --i, p++, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	if (p == 0) buf[i--] = '0'; // case for zero

	if (ngt)
		buf[i--] = '-';

	strcpy(buffer, &buf[i+1]);
	return buffer;
}

char* ultoa(unsigned long val, char* buffer, unsigned int base)
{
	int i = 34, p = 0;
	char buf[36] = {0};

	for(; val && i ; --i, p++, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	if (p == 0) buf[i--] = '0'; // case for zero

	strcpy(buffer, &buf[i+1]);
	return buffer;
}

// Author zitron: http://forum.arduino.cc/index.php?topic=37391#msg276209
// modif by ADiea: remove dependencies from strcat floor round
char *dtostrf(double floatVar, int minStringWidthIncDecimalPoint, int numDigitsAfterDecimal, char *outputBuffer)
{
	char temp[24], num[24];
	unsigned long mult = 1, int_part;
	int16_t i, processedFracLen = numDigitsAfterDecimal;
	double remainder;

	if (outputBuffer == NULL)
		return NULL ;

	char *buf = outputBuffer, *s;

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
		buf = num;

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

		floatVar += 0.5 / (float) mult;      // compute rounding factor

		int_part = (unsigned long) floatVar;

		s = ltoa(int_part, buf, 10);

		buf += strlen(s); //go to end of string


		if (numDigitsAfterDecimal != 0)
		{
			*buf++ = '.'; // print the decimal point

			long x = ((floatVar - int_part) * mult);

			s = ltoa( x, temp, 10);

			i = processedFracLen - strlen(s) + 1;

			while (--i > 0)
				*buf++ = '0';    // print padding zeros

			while (*s)
				*buf++ = *s++;

			i = numDigitsAfterDecimal - processedFracLen;

			while (i-- > 0)
				*buf++ = '0';    // ending fraction zeroes
		}

		//terminate num
		*buf = 0;
		buf = outputBuffer;

		// generate width space padding
		i = minStringWidthIncDecimalPoint - strlen(num);
		while (--i > 0)
		{
			*buf++ = ' ';
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
