/*
 * HardwareServo.h
 *
 *  Created on: 22.11.2015
 *      Author: johndoe
 */

#ifndef APP_HARDWARESERVO_H_
#define APP_HARDWARESERVO_H_
#include "../../SmingCore/SmingCore.h"
#include "HardwareServoChannel.h"

#define SERVO_CHANNEL_NUM_MAX 4


class HardwareServo
{
public:
	HardwareServo();
	virtual ~HardwareServo();

//	void Init();
//	bool SetValues(uint32 value[]);
	bool addChannel(HardwareServoChannel *channel);
	bool removeChannel(HardwareServoChannel *channel);
	void calcTiming();

private:
	void getPins();
	bool started = false;
	Vector<HardwareServoChannel*> channels;
};

extern HardwareServo hardwareServo;

#endif /* APP_HARDWARESERVO_H_ */
