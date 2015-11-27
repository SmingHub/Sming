/*
 * HardwareServo.h
 *
 *  Created on: 22.11.2015
 *      Author: johndoe
 */

#ifndef APP_HARDWARESERVO_H_
#define APP_HARDWARESERVO_H_
#include <SmingCore/SmingCore.h>

#define SERVO_CHANNEL_NUM_MAX 4

class HardwareServo
{
public:
	HardwareServo();
	HardwareServo(uint8 *pins, uint8 no_of_pins);
	virtual ~HardwareServo();

	void Init(uint8 *pins, uint8 no_of_pins);
	bool SetValues(uint32 value[]);

private:

	void calcTiming();

	TimerDelegate isrServo;
	uint8 channel_count;
	uint32 values[SERVO_CHANNEL_NUM_MAX];
};

extern HardwareServo hardwareServo;

#endif /* APP_HARDWARESERVO_H_ */
