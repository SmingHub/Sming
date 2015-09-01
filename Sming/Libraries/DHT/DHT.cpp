/*
 * Name: libDHT
 * License: MIT license.
 * Location: https://github.com/ADiea/libDHT
 * Maintainer: ADiea (https://github.com/ADiea)
 *
 * Descr: Arduino compatible DHT 11/22, AM2302 lib with dewpoint,
 *        heat-index and other goodies.
 *
 * Features:
 *  1. Autodetection of sensor type.
 *	2. Determine heat index.
 *	3. Determine dewpoint with various algorithms(speed vs accuracy).
 *	4. Determine thermal comfort:
 *		* Empiric comfort function based on comfort profiles(parametric lines)
 *		* Multiple comfort profiles possible. Default based on http://epb.apogee.net/res/refcomf.asp
 *		* Determine if it's too cold, hot, humid, dry, based on current comfort profile
 *  5. Should be compatible with both pre 1.0.1 and later Arduino versions - NOT TESTED
 *  6. Optimized for sensor read speed(~5ms for DHT22), stack and code size.
 *		*Select output between *C(smallest code size), *F, or runtime-defined via fct param.
 *	7. Compatible w/ Adafruit's lib but can also read both humidity and temp. at the same time.
 *
 * History:
 * 7/04/15 ADiea:	[experimental] comfort function; code reorganization; Autodetection;
 * 7/02/15 ADiea:	dew point algorithms
 * 6/25/15 ADiea: 	read temp and humidity in one function call
 *  	 	 	 	cache converted value for last temp and humid
 * 6/20/15 cloned from https://github.com/adafruit/DHT-sensor-library
 * -/--/-- written by Adafruit Industries
 *
 * License information:
 *
 * Copyright (c) 2015 ADiea
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "DHT.h"

void DHT::begin()
{
	//Pull the pin high to put the sensor in idle state
	pinMode(m_kSensorPin, OUTPUT);
	digitalWrite(m_kSensorPin, HIGH);

	//Make sure the first read() will happen
	m_lastreadtime = millis() - m_minIntervalRead;

	//Delay 250ms at least before the first read, so the sensor sees a stable
	//pin HIGH output
	delay(250);

	/*Autodetect sensor*/
	if(DHT_AUTO == m_kSensorType)
	{
		//set small wakeup to detect a DHT11 and start a read
		m_wakeupTimeMs = WAKEUP_DHT22;
		read();

		/*If no timeout error, must be a DHT22 type sensor*/
		if(m_lastError != errDHT_Timeout)
		{
			m_kSensorType = DHT22;
			if (READ_INTERVAL_DONT_CARE == m_minIntervalRead)
			{
				m_minIntervalRead = READ_INTERVAL_DHT22_DSHEET;
			}
			m_wakeupTimeMs = WAKEUP_DHT22;
			debugf("libDHT: Detected DHT-22 compatible sensor.");

			if(m_lastError == errDHT_OK)
				updateInternalCache();
		}
		else /* If sensor timedout it's probably a DHT11 */
		{
			m_kSensorType = DHT11;
			if (READ_INTERVAL_DONT_CARE == m_minIntervalRead)
			{
				m_minIntervalRead = READ_INTERVAL_DHT11_DSHEET;
			}
			m_wakeupTimeMs = WAKEUP_DHT11;
			debugf("libDHT: Detected DHT-11 compatible sensor.");
		}
	}
}

float DHT::readTemperature(
#if DHT_TEMPERATURE == 	DHT_RUNTIME
			bool bFarenheit/* = false*/
#endif
							)
{
	read();
#if ((DHT_TEMPERATURE == DHT_RUNTIME) || (DHT_TEMPERATURE == DHT_FARENHEIT))
	if ((NAN != m_lastTemp)
#if (DHT_TEMPERATURE == DHT_RUNTIME)
			&& bFarenheit
#endif
	)
		return convertCtoF(m_lastTemp);
#endif
	return m_lastTemp;
}


float DHT::readHumidity(void)
{
	read();
	return m_lastHumid;
}


bool DHT::readTempAndHumidity(TempAndHumidity& destReading
#if DHT_TEMPERATURE == 	DHT_RUNTIME
			, bool bFarenheit/* = false*/
#endif
)
{
	bool bSuccess = false;

	if (read())
	{
#if ((DHT_TEMPERATURE == DHT_RUNTIME) || (DHT_TEMPERATURE == DHT_FARENHEIT))
#if (DHT_TEMPERATURE == DHT_RUNTIME)
		if(bFarenheit)
#endif
			destReading.temp = convertCtoF(m_lastTemp);
#if (DHT_TEMPERATURE == DHT_RUNTIME)
		else
			destReading.temp = m_lastTemp;
#endif
#else
			destReading.temp = m_lastTemp;
#endif

		destReading.humid = m_lastHumid;

		bSuccess = true;
	}
	return bSuccess;
}


float DHT::getHeatIndex(float tempCelsius/*= LAST_VALUE*/,
						 float percentHumidity/*= LAST_VALUE*/
#if DHT_TEMPERATURE == 	DHT_RUNTIME
						 , bool bFarenheit/* = false*/
#endif
					)
{
	if(LAST_VALUE == tempCelsius)
	{
#if !NO_AUTOREFRESH
		if(!read())
			return NAN;
#endif
		tempCelsius = m_lastTemp;
		if(LAST_VALUE == percentHumidity)
		{
			percentHumidity = m_lastHumid;
		}
	}
	// Adapted from equation at: https://github.com/adafruit/DHT-sensor-library/issues/9 and
	// Wikipedia: http://en.wikipedia.org/wiki/Heat_index
	float t2C = tempCelsius * tempCelsius;
	float x = percentHumidity * percentHumidity;

	x = -8.784695 + 1.61139411 * tempCelsius + 2.33854900 * percentHumidity
			+ -0.14611605 * tempCelsius * percentHumidity + -0.01230809 * t2C
			+ -0.01642482 * x + 0.00221173 * t2C * percentHumidity
			+ 0.00072546 * tempCelsius * x + -0.00000358 * t2C * x;
#if ((DHT_TEMPERATURE == DHT_RUNTIME) || (DHT_TEMPERATURE == DHT_FARENHEIT))
#if (DHT_TEMPERATURE == DHT_RUNTIME)
	if(bFarenheit)
#endif
		x = convertCtoF(x);
#endif
	return x;
}


double DHT::getDewPoint(uint8_t algType /*= DEW_ACCURATE_FAST*/,
						float tempCelsius/*= LAST_VALUE*/,
						float percentHumidity/*= LAST_VALUE*/
#if DHT_TEMPERATURE == 	DHT_RUNTIME
						 , bool bFarenheit/* = false*/
#endif
		)
{
	double result = NAN;
	if(LAST_VALUE == tempCelsius)
	{
#if !NO_AUTOREFRESH
		if(!read())
		{
			return NAN;
		}

#endif
		tempCelsius = m_lastTemp;
		if(LAST_VALUE == percentHumidity)
		{
			percentHumidity = m_lastHumid;
		}
	}

	percentHumidity = percentHumidity * 0.01;

	switch(algType)
	{
		/*xx/xx/xxxx; x.xxxms @ xxMhz; Accuracy xx.xx; Platform xxxxxxx; Samples xxxx */
		/*Conclusion: */

	    /*04/07/2015; 1.210ms @ 80Mhz; Accuracy +0.00; Platform ESP8266; Samples 2100 */
		/*Conclusion: Accurate, resonably fast.
		 *            Tested on a few samples against http://www.decatur.de/javascript/dew/ */
		case DEW_ACCURATE:

		{
			/* 01/JUL/2015 ADiea: ported from FORTRAN http://wahiduddin.net/calc/density_algorithms.htm */
			/*
			FUNCTION ESGG(T)
			Baker, Schlatter  17-MAY-1982     Original version.
			THIS FUNCTION RETURNS THE SATURATION VAPOR PRESSURE OVER LIQUID
			WATER ESGG (MILLIBARS) GIVEN THE TEMPERATURE T (CELSIUS). THE
			FORMULA USED, DUE TO GOFF AND GRATCH, APPEARS ON P. 350 OF THE
			SMITHSONIAN METEOROLOGICAL TABLES, SIXTH REVISED EDITION, 1963,
			BY ROLAND LIST.
			*/
			double CTA = 273.15,  // DIFFERENCE BETWEEN KELVIN AND CELSIUS TEMPERATURES
				   EWS = 3.00571489795, // log10 of SATURATION VAPOR PRESSURE (MB) OVER LIQUID WATER AT 100C
				   TS = 373.15; // BOILING POINT OF WATER (K)

			double  C1 = -7.90298, C2 = 5.02808, C3 = 1.3816E-7, C4 = 11.344, C5 = 8.1328E-3,  C6 = -3.49149;
			tempCelsius = tempCelsius + CTA;
			result = (TS / tempCelsius) - 1;

			//   GOFF-GRATCH FORMULA

			result = pow(10, (C1 * result + C2 * log10(result + 1) -
						  C3 * (pow(10, (C4 * (1. - tempCelsius / TS))) - 1.) +
						  C5 * (pow(10, (C6 * result)) - 1.) + EWS));
			if(result < 0)
				result = 0;
			//result now holds the saturation vapor pressure in mBar
			//	https://en.wikipedia.org/wiki/Vapor_pressure
			//Convert from mBar to kPa (1mBar = 0.1 kPa) and divide by 0.61078 constant
			//Determine vapor pressure (takes the RH into account)
			//	http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
			result = percentHumidity * result / (10 * 0.61078);
			result = log(result);
			result =(241.88 * result) / (17.558 - result);
		}
		break;

	    /*xx/xx/xxxx; x.xxxms @ xxMhz; Accuracy xx.xx; Platform xxxxxxx; Samples xxxx */
		/*Conclusion: */

	    /*04/07/2015; 0.522ms @ 80Mhz; Accuracy -0.001; Platform ESP8266; Samples 2100 */
		/*Conclusion: Best choice, 0.001*C deviation with double speed */
		case DEW_ACCURATE_FAST:

		{
			/*Saturation vapor pressure is calculated by the datalogger
			 * with the following approximating polynomial
			 * (see Lowe, P.R. 1930. J. Appl. Meteor., 16:100-103):
			 * http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
			 */
			result = 6.107799961 +
						  tempCelsius * (0.4436518521 +
						  tempCelsius * (0.01428945805 +
						  tempCelsius * (2.650648471e-4 +
						  tempCelsius * (3.031240396e-6 +
						  tempCelsius * (2.034080948e-8 +
						  tempCelsius * 6.136820929e-11)))));
	        //Convert from mBar to kPa (1mBar = 0.1 kPa) and divide by 0.61078 constant
	        //Determine vapor pressure (takes the RH into account)
	        result = percentHumidity * result / (10 * 0.61078);
			result = log(result);
			result = (241.88 * result) / (17.558 - result);
		}
		break;

	    /*xx/xx/xxxx; x.xxxms @ xxMhz; Accuracy xx.xx; Platform xxxxxxx; Samples xxxx */
		/*Conclusion: */

	    /*04/07/2015; 0.723ms @ 80Mhz; Accuracy -0.06; Platform ESP8266; Samples 2100 */
		/*Conclusion: Worst choice. Very slow on this architecture, and inaccurate */
		case DEW_FAST:
		{
			/* 01/JUL/2015 ADiea: ported from FORTRAN http://wahiduddin.net/calc/density_algorithms.htm */
			/*
				Baker, Schlatter  17-MAY-1982     Original version.
				THIS FUNCTION RETURNS THE DEW POINT (CELSIUS) GIVEN THE TEMPERATURE
				(CELSIUS) AND RELATIVE HUMIDITY (%). THE FORMULA IS USED IN THE
				PROCESSING OF U.S. RAWINSONDE DATA AND IS REFERENCED IN PARRY, H.
				DEAN, 1969: "THE SEMIAUTOMATIC COMPUTATION OF RAWINSONDES,"
				TECHNICAL MEMORANDUM WBTM EDL 10, U.S. DEPARTMENT OF COMMERCE,
				ENVIRONMENTAL SCIENCE SERVICES ADMINISTRATION, WEATHER BUREAU,
				OFFICE OF SYSTEMS DEVELOPMENT, EQUIPMENT DEVELOPMENT LABORATORY,
				SILVER SPRING, MD (OCTOBER), PAGE 9 AND PAGE II-4, LINE 460.
			*/
				result = 1. - percentHumidity;

			/*  COMPUTE DEW POINT DEPRESSION. */
				result = (14.55 + 0.114 * tempCelsius)*result +
							 pow((2.5 + 0.007 * tempCelsius)*result, 3) +
							 (15.9 + 0.117 * tempCelsius)*pow(result, 14);

				result = tempCelsius - result;
		}
		break;

	    /*xx/xx/xxxx; x.xxxms @ xxMhz; Accuracy xx.xx; Platform xxxxxxx; Samples xxxx */
		/*Conclusion: */

	    /*04/07/2015; 0.522ms @ 80Mhz; Accuracy +0.03; Platform ESP8266; Samples 2100 */
		/*Conclusion: Bad choice. As fast as ACCURATEFAST but 30 times more inaccurate */
		case DEW_FASTEST:
		{
			/* http://en.wikipedia.org/wiki/Dew_point */
			double a = 17.271;
			double b = 237.7;
			result = (a * tempCelsius) / (b + tempCelsius) + log(percentHumidity);
			result = (b * result) / (a - result);
		}
		break;
	};

#if ((DHT_TEMPERATURE == DHT_RUNTIME) || (DHT_TEMPERATURE == DHT_FARENHEIT))
#if (DHT_TEMPERATURE == DHT_RUNTIME)
	if(bFarenheit)
#endif
	result = convertCtoF(result);
#endif
	return result;
}

float DHT::getComfortRatio(ComfortState& destComfortStatus,
		 float temperature/* = LAST_VALUE*/,
		 float percentHumidity/* = LAST_VALUE*/)
{
	if(LAST_VALUE == temperature)
	{
#if !NO_AUTOREFRESH
		if(!read())
			return NAN;
#endif
		temperature = m_lastTemp;
		if(LAST_VALUE == percentHumidity)
		{
			percentHumidity = m_lastHumid;
		}
	}
	float ratio = 100; //100%
	float distance = 0;
	float kTempFactor = 3; //take into account the slope of the lines
	float kHumidFactor = 0.1; //take into account the slope of the lines
	uint8_t tempComfort = 0;
	
	destComfortStatus = Comfort_OK;

	distance = m_comfort.distanceTooHot(temperature, percentHumidity);
	if(distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooHot;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kTempFactor;
	}
	
	distance = m_comfort.distanceTooHumid(temperature, percentHumidity);
	if(distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooHumid;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kHumidFactor;
	}	
	
	distance = m_comfort.distanceTooCold(temperature, percentHumidity);
	if(distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooCold;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kTempFactor;
	}

	distance = m_comfort.distanceTooDry(temperature, percentHumidity);
	if(distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooDry;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kHumidFactor;
	}

	destComfortStatus = (ComfortState)tempComfort;

	if(ratio < 0)
		ratio = 0;

	return ratio;
}

void DHT::updateInternalCache()
{
	/*Compute and write temp and humid to internal cache*/
	switch (m_kSensorType)
	{
		case DHT11:
			m_lastTemp = m_data[2];
			m_lastHumid = m_data[0];
			break;
		case DHT22:
		case DHT21:
			/*Temp*/
			m_lastTemp = ((uint32_t)(m_data[2] & 0x7F)<<8 | m_data[3]) / 10.0f;
			if (m_data[2] & 0x80)
			{
				m_lastTemp = -m_lastTemp;
			}
			/*Humidity*/
			m_lastHumid = (((uint32_t)m_data[0])<<8 | m_data[1]) / 10.0f;
			break;
		case DHT_AUTO:
			/*Sensor type unknown yet*/
			break;
		default:
			debugf("(update)libDHT: Unknown sensor type");
			break;
	}
}

bool DHT::read(void)
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;
	unsigned long time = millis();

	//Determine if it's appropiate to read the sensor, or return data from cache
	if ((time - m_lastreadtime) < m_minIntervalRead )
	{
		if (errDHT_OK == m_lastError)
			return true; // will use last data from cache
		else
		{
			return false; // must wait
		}
	}
	m_lastreadtime = time;

	//reset internal data and invalidate cache
	m_data[0] = m_data[1] = m_data[2] = m_data[3] = m_data[4] = 0;
	m_lastError = errDHT_Other;
	m_lastTemp = NAN;
	m_lastHumid = NAN;

	//Pull the pin low for m_wakeupTimeMs milliseconds
	pinMode(m_kSensorPin, OUTPUT);
	digitalWrite(m_kSensorPin, LOW);
	delay(m_wakeupTimeMs);
	//clear interrupts
	cli();
	//Make pin input and activate pullup
	PULLUP_PIN(m_kSensorPin);

	//Read in the transitions
	for (i = 0; i < MAXTIMINGS || j >= 40; i++)
	{
		counter = 0;
		while (digitalRead(m_kSensorPin) == laststate)
		{
			counter++;
			delayMicroseconds(1);
			if (counter == 255)
			{
				break;
			}
		}
		laststate = digitalRead(m_kSensorPin);

		if (counter == 255)
		{
			m_lastError = errDHT_Timeout;
			break;
		}

		// ignore first 3 transitions
		if ((i >= 4) && (i % 2 == 0))
		{
			// shove each bit into the storage bytes
			m_data[j / 8] <<= 1;

			if (counter > ONE_DURATION_THRESH_US)
			{
				m_data[j / 8] |= 1;
			}
			j++;
		}
	}
	sei();

#if DHT_DEBUG
	 Serial.println(j, DEC);
	 Serial.print(m_data[0], HEX); Serial.print(", ");
	 Serial.print(m_data[1], HEX); Serial.print(", ");
	 Serial.print(m_data[2], HEX); Serial.print(", ");
	 Serial.print(m_data[3], HEX); Serial.print(", ");
	 Serial.print(m_data[4], HEX); Serial.print(" =? ");
	 Serial.println((m_data[0] + m_data[1] + m_data[2] + m_data[3]) & 0xFF, HEX);
#endif

	// pull the pin high at the end
	 //(will stay high at least 250ms until the next reading)
	pinMode(m_kSensorPin, OUTPUT);
	digitalWrite(m_kSensorPin, HIGH);

	// check we read 40 bits and that the checksum matches
	if ((j >= 40) &&
	    (m_data[4] == ((m_data[0] + m_data[1] + m_data[2] + m_data[3]) & 0xFF)))
	{
		updateInternalCache();
		m_lastError = errDHT_OK;
		return true;
	}
	else
	{
		m_lastError = errDHT_Checksum;
	}

	return false;
}
