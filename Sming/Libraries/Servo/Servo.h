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
#ifndef LIB_SERVO_H_
#define LIB_SERVO_H_
#include <SmingCore.h>
#include "ServoChannel.h"

#define SERVO_CHANNEL_NUM_MAX 4 ///< maximum number of servo channels

class Servo
{
public:
	/** @brief  Instantiate servo object\n\r
     * public global instance of Servo is available as variable servo\n\r
     * no additional instance is allowed because ServoChannel instances use servo
     */
	Servo();
	virtual ~Servo();

	//	void Init();
	//	bool SetValues(uint32 value[]);

	/** @brief  add a servo channel to the servo object\n\r
     * locks interrupt to change configuration without timing problem, may result in a chort glitch
     *  @param  channel pointer to the servo channel object
     *  @retval bool True on success
     */
	bool addChannel(ServoChannel* channel);

	/** @brief  remove a servo channel from the servo object\n\r
     * locks interrupt to change configuration without timing problem, may result in a chort glitch
     *  @param  channel pointer to the servo channel object
     *  @retval bool True on success
     */
	bool removeChannel(ServoChannel* channel);

	/** @brief  calculation of all the internal timings\n\r
     * has to be called when min of max is changed and setDegree is used
     */
	void calcTiming();

private:
	void getPins();
	Vector<ServoChannel*> channels;
};

extern Servo servo; ///< global instance of the servo object
/** @} */

#endif /* LIB_SERVO_H_ */
