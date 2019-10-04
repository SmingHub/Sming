/*
 * Servo.h
 *
 *  Created on: 22.11.2015
 *      Author: johndoe
 */
/** @addtogroup   Servo RC Servo functions
 *  @brief      Provides Library to control rc servos with pwm signals
 *  - uses an internal instance of HardwareTimer (which use interrupts)
 *  @{
*/

#pragma once

#include "ServoChannel.h"
#include <HardwareTimer.h>

#define SERVO_CHANNEL_NUM_MAX 4 ///< maximum number of servo channels

class Servo
{
public:
	/** @brief  Instantiate servo object
	 *  @note   Public global instance of Servo is available as variable servo.
     * 	No additional instance is allowed because ServoChannel instances use servo.
     */
	Servo();

	virtual ~Servo()
	{
	}

	//	void Init();
	//	bool SetValues(uint32 value[]);

	/** @brief  add a servo channel to the servo object
     *  @param  channel pointer to the servo channel object
     *  @retval bool True on success
     *  @note   Locks interrupt to change configuration without timing problem, may result in a short glitch
     */
	bool addChannel(ServoChannel* channel);

	/** @brief  remove a servo channel from the servo object
     *  @param  channel pointer to the servo channel object
     *  @retval bool True on success
     *  @note   Locks interrupt to change configuration without timing problem, may result in a short glitch
     */
	bool removeChannel(ServoChannel* channel);

	/** @brief  calculation of all the internal timings
     *  @note   Has to be called when min of max is changed and setDegree is used
     */
	void calcTiming();

private:
	void getPins();
	static void IRAM_ATTR timerInt();

private:
	Vector<ServoChannel*> channels;
	static HardwareTimer hardwareTimer;
	static uint8 pins[SERVO_CHANNEL_NUM_MAX];
	static uint32 timing[SERVO_CHANNEL_NUM_MAX * 2 + 1];
	static uint8 maxTimingIdx;
	static uint8 actIndex;
	static bool started;
};

extern Servo servo; ///< global instance of the servo object
/** @} */
