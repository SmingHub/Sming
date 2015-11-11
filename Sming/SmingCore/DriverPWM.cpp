/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/


#include <limits.h>
#include "DriverPWM.h"
#include "Digital.h"

DriverPWM::DriverPWM() : initialized(false)
{
	period = 2000;
	channels.add(ChannelPWM(0));
}

void DriverPWM::initialize()
{
	if (!initialized)
	{
		initialized = true;
		os_timer_disarm(&main);
		os_timer_setfn(&main, (os_timer_func_t *)processingStatic, this);
		ets_timer_arm_new(&main, period, 1, 0);
	}
}

void DriverPWM::analogWrite(uint8_t pin, int duty)
{
	int work = -1;
	for (int i = 0; i < channels.count() && work == -1; i++)
		if (channels[i].id() == pin)
			work = i;

	if (work == -1) // new channel
	{
		channels.add(ChannelPWM(pin));
		work = channels.count() - 1;
	}

	channels[work].config(duty, period);
}

void DriverPWM::noAnalogWrite(uint8_t pin)
{
	for (int i = 0; i < channels.count(); i++)
	{
		if (channels[i].id() == pin)
		{
			channels[i].close();
			channels.remove(i);
		}
	}

	if (channels.count() == 0)
	{
		os_timer_disarm(&main);
		initialized = false;
	}
}

void DriverPWM::processingStatic(void *arg)
{
	DriverPWM *self = (DriverPWM*)arg;
	int k = self->channels.count();
	for (int i = 0; i < k; i++)
		self->channels[i].high();
}

//////////////////////////

ChannelPWM::ChannelPWM() : pin(-1), time(0)
{
}

ChannelPWM::ChannelPWM(int pwmPin)
	: pin(pwmPin), time(0)
{
	pinMode(pin, OUTPUT);
	digitalWrite(pin, 0);
	initialize();
}

void ChannelPWM::initialize()
{
	os_timer_disarm(&item);
	os_timer_setfn(&item, (os_timer_func_t *)processingStatic, this);
}

void ChannelPWM::config(int duty, uint32_t basePeriod)
{
	if (duty < 1)
		duty = 0;
	else if (duty > PWM_DEPTH)
		duty = PWM_DEPTH;

	time = basePeriod * duty / PWM_DEPTH;

	if (time >= basePeriod) // Full ON mode
		time = ULONG_MAX;
}

void ChannelPWM::high()
{
	if (time == 0) return; // Full OFF mode

	digitalWrite(pin, HIGH);

	if (time != ULONG_MAX) // Full ON mode
	{
		os_timer_disarm(&item);
		item.timer_arg = this;
		ets_timer_arm_new(&item, time, 0, 0);
	}
}

void ChannelPWM::close()
{
	os_timer_disarm(&item);
	digitalWrite(pin, 0);
}

void ChannelPWM::processingStatic(void *arg)
{
	ChannelPWM *self = (ChannelPWM*)arg;
	digitalWrite(self->pin, LOW);
}
