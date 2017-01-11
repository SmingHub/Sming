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

#ifndef LIB_SERVOCHANNEL_H_
#define LIB_SERVOCHANNEL_H_

#include "../../SmingCore/SmingCore.h"

#define DEFAULTMAXVALUE 2300	///< default max value for output in us
#define DEFAULTMINVALUE 700		///< default min value for output in us


class ServoChannel
{
public:
    /** @brief  Instantiate a servo channel object\n\r
     * has to be attached to a pin before using
     */
	ServoChannel();
	virtual ~ServoChannel();

    /** @brief  attach servo channel to a unused pin
     *  @param  pin GPIO to use\n\r
     * pin will be set to output
     *  @retval bool True on success
     */
	bool attach(uint8 pin);
    /** @brief  detaches servo channel
     * pin will be set to input
     *  @retval bool True on success
     */
	bool detach();

    /** @brief  set value in servo degree\n\r
     * input value is mapped between min and max value
     *  @param  value is limited between -90 and 90 degrees
     *  @retval bool True on success
     */
	bool setDegree(int8 value);

    /** @brief  set max value for output timing
     *  @param  value time in us for maximum output of this channel\n\r
     *  has to be larger than min value
     *  @retval bool True on success
     */
	bool setMaxValue(uint32 maxValue = DEFAULTMAXVALUE);

    /** @brief  set min value for output timing
     *  @param  value time in us for minimum output of this channel\n\r
     *  has to be smaller than max value
     *  @retval bool True on success
     */
	bool setMinValue(uint32 minValue = DEFAULTMINVALUE);

    /** @brief  simple getter
     *  @retval uint32 max time in us
     */
	uint32 getMaxValue() const { return maxValue; }

    /** @brief  simple getter
     *  @retval uint32 min time in us
     */
	uint32 getMinValue() const { return minValue; }

    /** @brief  simple getter
     *  @retval uint32 actual value in us
     */
	uint32 getValue() const;

    /** @brief  set value for output
     *  @param  value time in us for actual output of this channel\n\r
     *  has to be smaller than max-min value
     *  @retval bool True on success
     */
	bool setValue(uint32 value);

    /** @brief  simple getter
     *  @retval uint8 attached pin for this channel
     */
	uint8 getPin() const { return pin; }

private:
	void plausValue();
	uint8 pin;
	uint32 value;
	uint32 minValue;
	uint32 maxValue;
};
/** @} */

#endif /* LIB_SERVOCHANNEL_H_ */
