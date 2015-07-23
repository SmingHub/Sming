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

char* ftoa(char *buf, float f)
{

	size_t n = 0;
	uint8_t digits = 6;

	do
	{

	  if (isnan(f))
	  {
		  *buf++ = 'n'; *buf++ = 'a'; *buf++ = 'n';
		  break;
	  }
	  if (isinf(f))
	  {
		  *buf++ = 'i'; *buf++ = 'n'; *buf++ = 'f';
		  break;
	  }
	  if (f > 4294967040.0)
	  {
		  *buf++ = 'o'; *buf++ = 'v'; *buf++ = 'f';
		  break;
	  }
	  if (f <-4294967040.0)
	  {
		  *buf++ = '-'; *buf++ = 'o'; *buf++ = 'v'; *buf++ = 'f';
		  break;
	  }

	  // Handle negative numbers
	  if (f < 0.0)
	  {
		  *buf++ = '-';
	    f = -f;
	  }

	  // Round correctly so that print(1.999, 2) prints as "2.00"
	  double rounding = 0.0000005;

	  f += rounding;

	  // Extract the integer part of the number and print it
	  unsigned long int_part = (unsigned long)f;
	  double remainder = f - (double)int_part;

	  buf = ee_number(buf, int_part, 10, 0);

	  // Print the decimal point, but only if there are digits beyond
	  *buf++ = '.';

	  // Extract digits from the remainder one at a time
	  while (digits-- > 0)
	  {
	    remainder *= 10.0;
	    uint8_t toPrint = uint8_t(remainder);
	    *buf++ = toPrint + '0';
	    remainder -= toPrint;
	  }


		for (--buf; buf[0] == '0' && buf[-1] != '.'; --buf)
			;
		++buf;

	}
	while(0);

	return buf;
}


//extern char * ets_strcat ( char * destination, const char * source );
//#define strcat ets_strcat
// Author zitron: http://forum.arduino.cc/index.php?topic=37391#msg276209
//some modif by ADiea
char *dtostrf(double floatVar, int minStringWidthIncDecimalPoint, int numDigitsAfterDecimal, char *outputBuffer)
{
	 if (outputBuffer == NULL) return NULL;

	 char *buf = outputBuffer;

	 *outputBuffer = 0;

	 if (isnan(floatVar))
	 {*buf++ = 'N'; *buf++ = 'a'; *buf++ = 'N';}
	 else if (isinf(floatVar))
	 {*buf++ = 'I'; *buf++ = 'n'; *buf++ = 'f';}
	 else if (floatVar > 4294967040.0)
	 {*buf++ = 'O'; *buf++ = 'V'; *buf++ = 'F';}  // constant determined empirically
	 else if (floatVar <-4294967040.0)
	 {*buf++ = 'o'; *buf++ = 'v'; *buf++ = 'f';}  // constant determined empirically

	 if (*outputBuffer != 0)
	 {
		 *buf++ = 0;
		 return outputBuffer;
	 }

	 char temp[24];
	 int16_t i;


	 if(floatVar < 0.0){
		*buf++ = '-';  //print "-" sign
	   floatVar *= -1;
	 }

	 if( numDigitsAfterDecimal == 0) {
	   strcat(outputBuffer, ltoa(round(floatVar), temp, 10));  //prints the int part
	 }
	 else {
	   unsigned long frac, mult = 1;
	   int16_t  processedFracLen = numDigitsAfterDecimal;
	   if (processedFracLen > 9) processedFracLen = 9; // Prevent overflow!

	   int16_t padding = processedFracLen - 1;

	   int16_t k = processedFracLen;
	   while (k-- > 0)
	     mult = mult<<3 + mult<<2; //mult *= 10

	   floatVar += 0.5/(float)mult;      // compute rounding factor

	   strcat(outputBuffer, ltoa(floor(floatVar), temp, 10));  //prints the integer part without rounding
	   strcat(outputBuffer, ".\0"); // print the decimal point

	   frac = (floatVar - floor(floatVar)) * mult;

	   unsigned long frac1 = frac;

	   while(frac1 /= 10)
	     padding--;

	   while(padding--)
	     strcat(outputBuffer,"0\0");    // print padding zeros

	   strcat(outputBuffer,ltoa(frac,temp,10));  // print fraction part

	   k = numDigitsAfterDecimal - processedFracLen;
	   while (k-- > 0)
	     strcat(outputBuffer,"0\0"); // ending fraction zeroes
	 }

	 // generate width space padding
	 if ((minStringWidthIncDecimalPoint != 0) && (minStringWidthIncDecimalPoint >= strlen(outputBuffer))){
	   int16_t J=0;
	   J = minStringWidthIncDecimalPoint - strlen(outputBuffer);

	   for (i=0; i< J; i++) {
	     temp[i] = ' ';
	   }

	   temp[i++] = '\0';
	   strcat(temp,outputBuffer);
	   strcpy(outputBuffer,temp);
	 }

	 return outputBuffer;
}
