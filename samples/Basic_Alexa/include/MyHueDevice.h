#pragma once

#include <Hue/Device.h>
#include <Timer.h>
#include <HardwareSerial.h>

/*
 * Demonstration of creating a custom Hue device with asynchronous request handling.
 */
class MyHueDevice : public Hue::Device
{
public:
	using Status = Hue::Status;

	MyHueDevice(ID id, const String& name) : id(id), name(name)
	{
	}

	ID getId() const override
	{
		return id;
	}

	String getName() const override
	{
		return name;
	}

	bool getAttribute(Attribute attr, unsigned& value) const override
	{
		switch(attr) {
		case Attribute::on:
			value = on;
			return true;
		default:
			return false;
		}
	}

	Status setAttribute(Attribute attr, unsigned value, Callback callback) override
	{
		// Pend this request for 2 seconds
		auto timer = new AutoDeleteTimer;
		auto action = new Action{attr, value, callback};
		timer->initializeMs<2000>([this, action]() {
			Status status;
			if(action->attr == Attribute::on) {
				this->on = (action->value != 0);
				status = Status::success;
			} else {
				status = Status::error;
			}
			Serial.println(_F("Completing MyDevice::setAttribute"));
			action->callback(status, 0);
			delete action;
		});
		timer->startOnce();
		Serial.println(_F("Pending MyDevice::setAttribute"));
		return Status::pending;
	}

private:
	struct Action {
		Attribute attr;
		unsigned value;
		Callback callback;
	};

	ID id;
	String name;
	bool on{false};
};
