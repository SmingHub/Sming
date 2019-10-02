/*
 * Created by SharpDevelop.
 * User: Tefik Becirovic
 * Date: 15.10.2008
 * Time: 19:42
 * 
 */

#include "include/SignalGenerator.h"
#include "StatisticFunction.h"
#include <stdlib.h>
#include <esp_systemapi.h>

#undef M_PI
const double M_PI = 3.1415926535897932384626433832795;

const uint32_t ticksPerSecond = 1000000;

float SignalGenerator::getValue(float time)
{
	float value = 0.0;
	float t = frequency * time + phase;
	switch(signalType) { // http://en.wikipedia.org/wiki/Waveform
	case eST_Sine:		 // sin( 2 * pi * t )
		value = sin(2.0 * M_PI * t);
		break;
	case eST_Square: // sign( sin( 2 * pi * t ) )
		value = sin(2.0 * M_PI * t) < 0 ? -1 : 1;
		break;
	case eST_Triangle: // 2 * abs( t - 2 * floor( t / 2 ) - 1 ) - 1
		value = 1.0 - 4.0 * abs(round(t - 0.25) - (t - 0.25));
		break;
	case eST_Sawtooth: // 2 * ( t/a - floor( t/a + 1/2 ) )
		value = 2.0 * (t - floor(t + 0.5));
		break;

	case eST_Pulse: // http://en.wikipedia.org/wiki/Pulse_wave
		value = (abs(sin(2 * M_PI * t)) < 1.0 - 10E-3) ? 0 : 1;
		break;
	case eST_WhiteNoise: // http://en.wikipedia.org/wiki/White_noise
		value = (2.0 * os_random() / 0xffffffff) - 1.0;
		break;
	case eST_GaussNoise: // http://en.wikipedia.org/wiki/Gaussian_noise
		value = StatisticFunction::NORMINV((float)os_random() / 0xffffffff, 0.0, 0.4);
		break;
	case eST_DigitalNoise: // Binary Bit Generators
		value = (os_random() & 1) ? 1.0 : -1.0;
		break;

	case eST_UserDefined:
		value = getValueCallback ? getValueCallback(t) : 0.0;
		break;
	}

	return ((invert ? 1.0 : -1.0) * amplitude * value) + offset;
}

float SignalGenerator::getValue()
{
	float time = float(micros() - startTime) / ticksPerSecond;
	return getValue(time);
}
