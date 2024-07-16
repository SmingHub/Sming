ESP32 BLE Gamepad
=================

.. highlight:: c++

Introduction
------------
This library allows you to make the ESP32 act as a Bluetooth gamepad and control what it does.  
The library uses :library:`NimBLE` for faster and lighter communication.

Features
--------

Using this library you can do the following:

 - Button press (128 buttons)
 - Button release (128 buttons)
 - Axes movement (6 axes (16 bit) (x, y, z, rZ, rX, rY) --> (Left Thumb X, Left Thumb Y, Right Thumb X, Right Thumb Y, Left Trigger, Right Trigger))
 - 2 Sliders (16 bit) (Slider 1 and Slider 2)
 - 4 point of view hats (ie. d-pad plus 3 other hat switches)
 - Simulation controls (rudder, throttle, accelerator, brake, steering)
 - Configurable HID descriptor
 - Report optional battery level to host (basically works, but it doesn't show up in Android's status bar)
 - Customize Bluetooth device name/manufacturer
 - Uses efficient NimBLE bluetooth library
 - Compatible with Windows
 - Compatible with Android (Android OS maps default buttons / axes / hats slightly differently than Windows)
 - Compatible with Linux (limited testing)
 - Compatible with MacOS X (limited testing)

Using
-----

1. Add ``COMPONENT_DEPENDS += BLEGamepad`` to your application component.mk file.
2. Add these lines to your application::

	#include <BleGamepad.h>
	
	namespace
	{
		BleGamepad bleGamepad;
	
		// ...
	
	} // namespace
		
	void init()
	{
		// ...
		
		bleGamepad.begin();
	}


Notes
-----
By default, reports are sent on every button press/release or axis/slider/hat/simulation movement, however this can be disabled, 
and then you manually call sendReport on the gamepad instance as shown in the IndividualAxes.ino example.

There is also Bluetooth specific information that you can use (optional):

Instead of ``BleGamepad bleGamepad;`` you can do ``BleGamepad bleGamepad("Bluetooth Device Name", "Bluetooth Device Manufacturer", 100);``.
The third parameter is the initial battery level of your device. 
Adjusting the battery level later on doesn't work.
By default the battery level will be set to 100%, the device name will be `ESP32 BLE Gamepad` and the manufacturer will be `Espressif`.