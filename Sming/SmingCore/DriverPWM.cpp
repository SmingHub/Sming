/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include <limits.h>
#include "DriverPWM.h"
#include "Digital.h"

DriverPWM::DriverPWM() : _initialized(false)
{
	_period = 2000;
	_channels.add(ChannelPWM(0));
}

void DriverPWM::initialize()
{
	if (!_initialized) {
		_initialized = true;
		os_timer_disarm(&_main);
		os_timer_setfn(&_main, (os_timer_func_t*)processingStatic, this);
		ets_timer_arm_new(&_main, _period, 1, 0);
	}
}

void DriverPWM::analogWrite(uint8_t pin, int duty)
{
	int work = -1;
	for (unsigned i = 0; i < _channels.count() && work == -1; i++)
		if (_channels[i].id() == pin)
			work = i;

	// new channel
	if (work == -1) {
		_channels.add(ChannelPWM(pin));
		work = _channels.count() - 1;
	}

	_channels[work].config(duty, _period);
}

void DriverPWM::noAnalogWrite(uint8_t pin)
{
	for (unsigned i = 0; i < _channels.count(); i++) {
		if (_channels[i].id() == pin) {
			_channels[i].close();
			_channels.remove(i);
		}
	}

	if (_channels.count() == 0) {
		os_timer_disarm(&_main);
		_initialized = false;
	}
}

void DriverPWM::processingStatic(void* arg)
{
	auto self = reinterpret_cast<DriverPWM*>(arg);
	for (unsigned i = 0; i < self->_channels.count(); i++)
		self->_channels[i].high();
}

//////////////////////////

ChannelPWM::ChannelPWM() : _pin(-1), _time(0)
{}

ChannelPWM::ChannelPWM(int pwmPin) : _pin(pwmPin), _time(0)
{
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, 0);
	initialize();
}

void ChannelPWM::initialize()
{
	os_timer_disarm(&_item);
	os_timer_setfn(&_item, processingStatic, this);
}

void ChannelPWM::config(int duty, uint32_t basePeriod)
{
	if (duty < 1)
		duty = 0;
	else if (duty > PWM_DEPTH)
		duty = PWM_DEPTH;

	_time = basePeriod * duty / PWM_DEPTH;

	// Full ON mode
	if (_time >= basePeriod)
		_time = ULONG_MAX;
}

void ChannelPWM::high()
{
	// Full OFF mode
	if (_time == 0)
		return;

	digitalWrite(_pin, HIGH);

	// Full ON mode
	if (_time != ULONG_MAX) {
		os_timer_disarm(&_item);
		_item.timer_arg = this;
		ets_timer_arm_new(&_item, _time, 0, 0);
	}
}

void ChannelPWM::close()
{
	os_timer_disarm(&_item);
	digitalWrite(_pin, 0);
}

void ChannelPWM::processingStatic(void* arg)
{
	auto self = reinterpret_cast<ChannelPWM*>(arg);
	digitalWrite(self->_pin, LOW);
}
