At Client
=========

.. highlight:: c++

Introduction
------------

AT commands were initially instructions used to control a modem. AT is the abbreviation of ATtention. 
Every command line starts with "AT" or "at". 
If interested a good background article is `Hayes Command Set <https://en.wikipedia.org/wiki/Hayes_command_set>`__. 

Nowadays also other devices allow communication via AT commands as for example GSM/GPRS modems, GPS trackers, web cameras and more.

This library simplifies the communication with such devices.

Usage
-----

1. Add ``COMPONENT_DEPENDS += AtClient`` to your application component.mk file.
2. Add these lines to your application::

	#include <AtClient.h>
	
	namespace
	{
		AtClient* atClient;
	
		// ...
	
	} // namespace
		
	void init()
	{
		Serial.begin(SERIAL_BAUD_RATE);
		Serial.systemDebugOutput(true);

		atClient = new AtClient(Serial);
		atClient->send("ATE0\r");
		// ... 
	}
