#ifndef __H_ARDUINO_WEATHER__
#define __H_ARDUINO_WEATHER__

namespace weather {
	// Celsius to Fahrenheit conversion
	inline
	double fahrenheit(double celsius) {
		return 1.8 * celsius + 32;
	}

	// fast integer version with rounding
	inline
	int fahrenheitFast(int celcius) {
		return (celcius * 18 + 5)/10 + 32;
	}


	// Celsius to Kelvin conversion
	inline
	double kelvin(double celsius) {
		return celsius + 273.15;
	}

	// dewPoint function NOAA
	// reference: http://wahiduddin.net/calc/density_algorithms.htm 
	inline
	double dewPoint(double celsius, double humidity) {
		// RATIO was originally named A0, possibly confusing in Arduino context
		double RATIO = 373.15 / (273.15 + celsius);
		double SUM = -7.90298 * (RATIO - 1);
		SUM += 5.02808 * log10(RATIO);
		SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
		SUM += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
		SUM += log10(1013.246);
		double VP = pow(10, SUM - 3) * humidity;
		double T = log(VP/0.61078);   // temp var
		return (241.88 * T) / (17.558 - T);
	}

	// delta max = 0.6544 wrt dewPoint()
	// 5x faster than dewPoint()
	// reference: http://en.wikipedia.org/wiki/Dew_point
	inline
	double dewPointFast(double celsius, double humidity) {
		double a = 17.271;
		double b = 237.7;
		double temp = (a * celsius) / (b + celsius) + log(humidity/100);
		double Td = (b * temp) / (a - temp);
		return Td;
	}
};

#endif