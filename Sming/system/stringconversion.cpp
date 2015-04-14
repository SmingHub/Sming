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
char *dtostrf(double floatVar, int minStringWidthIncDecimalPoint, int numDigitsAfterDecimal, char *outputBuffer)
{
	 if (outputBuffer == NULL) return NULL;

	 *outputBuffer = 0;

	 if (isnan(floatVar))
		 strcpy(outputBuffer, "NaN");
	 else if (isinf(floatVar))
		 strcpy(outputBuffer, "Inf");
	 else if (floatVar > 4294967040.0)
		 strcpy(outputBuffer, "OVF");  // constant determined empirically
	 else if (floatVar <-4294967040.0)
		 strcpy(outputBuffer, "OVF");  // constant determined empirically

	 if (*outputBuffer != 0)
		 return outputBuffer;

	 char temp[24];
	 int16_t i;

	 temp[0]='\0';
	 outputBuffer[0]='\0';

	 if(floatVar < 0.0){
	   strcpy(outputBuffer,"-\0");  //print "-" sign
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
	     mult *= 10;

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
