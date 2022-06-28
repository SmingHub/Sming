Switch Joycon
=============

.. highlight:: c++

Introduction
------------
This library allows you to make the ESP32 act as a Nintendo Switch Joycon and control what it does.  
The library uses :library:`NimBLE` for faster and lighter communication.

Disclaimer
----------
We are not affiliated, associated, authorized, endorsed by, or in any way officially connected with Nintendo, 
or any of its subsidiaries or its affiliates. 
The names Nintendo, Nintendo Switch and Joycon as well as related names, marks, emblems and images are 
registered trademarks of their respective owners.

Features
--------

Using this library you can do the following:

 - Button press and release (16 buttons)
 - Switch Hat (1 hat )
 - Rotate 4 Axis

Using
-----

1. Add ``COMPONENT_DEPENDS += SwitchJoycon`` to your application componenent.mk file.
2. Add these lines to your application::

	#include <SwitchJoycon.h>
	
	namespace
	{
		SwitchJoycon joycon;
	
		// ...
	
	} // namespace
		
	void init()
	{
		// ...
		
		joycon.begin();
	}


Notes
-----
By default, reports are sent on every button press/release or axis/hat movement, however this can be disabled::

	joycon.setAutoReport(false);
 
and then you should manually call sendReport on the joycon instance as shown below::

	joycon.sendReport();


HID Debugging
-------------

On Linux you can install `hid-tools <https://gitlab.freedesktop.org/libevdev/hid-tools>`__ using the command below::

	sudo pip3 install .

Once installed hid-recorder can be used to check the device HID report description and sniff the different reports::

	sudo hid-recorder

Useful Links
------------
- `Tutorial about USB HID Report Descriptors <https://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/>`__
- `HID constants <https://github.com/katyo/hid_def/blob/master/include/hid_def.h>`__
