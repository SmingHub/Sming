/*
 * Created by SharpDevelop.
 * User: Tefik Becirovic
 * Date: 15.10.2008
 * Time: 19:42
 *
 * https://www.codeproject.com/Articles/30180/Simple-Signal-Generator
 *
 * 10/8/19 mikee47 ported from C#
 */

#pragma once

#include <functional>
#include <Clock.h>

enum SignalType {
	eST_Sine,
	eST_Square,
	eST_Triangle,
	eST_Sawtooth,

	eST_Pulse,
	eST_WhiteNoise,   ///< random
	eST_GaussNoise,   ///< random with normal distribution
	eST_DigitalNoise, ///< random hard-limited output

	eST_UserDefined,
};

typedef std::function<float(float time)> SignalGetValueDelegate;

class SignalGenerator
{
public:
	/**
	 * @brief Type of signal to generate
	 */
	SignalType signalType = eST_Sine;

	/**
	 * @brief Signal frequency in Hz
	 */
	float frequency = 2000.0;

	/**
	 * @brief Signal phase, offset applied in seconds
	 */
	float phase = 0.0;

	/**
	 * @brief Signal amplitude (peak value)
	 */
	float amplitude = 1.0;

	/**
	 * @brief Signal offset applied to output
	 */
	float offset = 0.0;

	/**
	 * @brief Whether to invert output
	 */
	bool invert = false;

	/**
	 * @brief System time when signal started
	 */
	uint32_t startTime;

	/**
	 * @brief Callback to obtain signal value when signalType is eST_UserDefined
	 */
	SignalGetValueDelegate getValueCallback = nullptr;

	/**
	 * @brief Construct a SignalGenerator with useful output
	 */
	SignalGenerator(SignalType initialSignalType = eST_Sine, float frequency = 2.0)
		: signalType(initialSignalType), frequency(frequency)
	{
		reset();
	}

	/**
	 * @brief Get the output value at a specific point in time
	 * @param time In seconds
	 * @retval Signal generator output value
	 */
	float getValue(float time);

	/**
	 * @brief Get the output value corresponding to the current point in time
	 * @retval Output value
	 */
	float getValue();

	/**
	 * @brief Set the start time
	 * @param microseconds Time to
	 * @note
	 */
	void setStartTime(uint32_t microseconds)
	{
		startTime = microseconds;
	}

	/**
	 * @brief Set start time to current system time
	 */
	void reset()
	{
		startTime = micros();
	}
};
