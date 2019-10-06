/*
 * ServoChannel.h
 *
 *  Created on: 27.11.2015
 *      Author: johndoe
 */
/** @addtogroup   Servo RC Servo functions
 *  @brief      Provides Library to control rc servos with pwm signals
 *  @{
*/

#pragma once

#include <stdint.h>

/**
 * @brief Manages a single servo channel
 * @note The actual positioning is handled by the Servo class, so for further details have a look there.
 */
class ServoChannel
{
public:
	static constexpr uint32_t defaultMinValue = 700;  ///< default min value for output in us
	static constexpr uint32_t defaultMaxValue = 2300; ///< default max value for output in us

	/** @brief  Instantiate a servo channel object
     *  @note has to be attached to a pin before using
     */
	ServoChannel()
	{
	}

	virtual ~ServoChannel()
	{
	}

	/** @brief  attach servo channel to a unused pin
     *  @param  pin GPIO to use; pin will be set to output
     *  @retval bool True on success
     */
	bool attach(uint8_t pin);

	/** @brief  detaches servo channel; pin will be set to input
     *  @retval bool True on success
     */
	bool detach();

	/** @brief  Set value in servo degree
     *  @param  Value Limited between -90 and 90 degrees
     *          Mapped between min and max value.
     *  @retval uint32_t Value in microseconds actually set
     */
	uint32_t setDegree(int8_t value);

	/** @brief  Set maximum value for output timing
     *  @param  maxValue Time in microseconds for maximum output of this channel.
     *          Has to be larger than min value.
     *  @retval bool true on success, false if out of range
     */
	bool setMaxValue(uint32_t maxValue = defaultMaxValue);

	/** @brief  Set minimum value for output timing
     *  @param  minValue Time in microseconds for minimum output of this channel
     *          Has to be smaller than max value
     *  @retval bool true on success, false if out of range
     */
	bool setMinValue(uint32_t minValue = defaultMinValue);

	/** @brief  Get maximum time value
     *  @retval uint32_t Maximum time in microseconds
     */
	uint32_t getMaxValue() const
	{
		return maxValue;
	}

	/** @brief  Get minimum time value
     *  @retval uint32_t Minimum time in microseconds
     */
	uint32_t getMinValue() const
	{
		return minValue;
	}

	/** @brief  simple getter
     *  @retval uint32_t actual value in us
     */
	uint32_t getValue() const
	{
		return value;
	}

	/** @brief  set value for output
     *  @param  value Time in us for actual output of this channel.
     *          Will be constrained to value min-max range.
     *  @retval uint32_t Actual value set
     */
	uint32_t setValue(uint32_t value);

	/** @brief  simple getter
     *  @retval uint8_t attached pin for this channel
     */
	uint8_t getPin() const
	{
		return pin;
	}

private:
	void update();

private:
	uint8_t pin = 0;
	uint32_t value = (defaultMinValue + defaultMaxValue) / 2;
	uint32_t minValue = defaultMinValue;
	uint32_t maxValue = defaultMaxValue;
};
/** @} */
