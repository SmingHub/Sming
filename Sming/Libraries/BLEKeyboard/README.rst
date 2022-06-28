ESP32 BLE Keyboard
==================

.. highlight:: c++

Introduction
------------
This library allows you to make the ESP32 act as a Bluetooth keyboard and control what it does.  
The library uses :library:`NimBLE` for faster and lighter communication.

Features
--------

Using this library you can do the following:

 - Send key strokes
 - Send text
 - Press/release individual keys
 - Media keys are supported
 - Set battery level (basically works, but doesn't show up in Android's status bar)
 - Compatible with Android
 - Compatible with Windows
 - Compatible with Linux
 - Compatible with MacOS X (not stable, some people have issues, doesn't work with old devices)
 - Compatible with iOS (not stable, some people have issues, doesn't work with old devices)

Using
-----

1. Add ``COMPONENT_DEPENDS += BLEKeyboard`` to your application componenent.mk file.
2. Add these lines to your application::

	#include <BleKeyboard.h>

	
	namespace
	{
		BleKeyboard bleKeyboard;
	
		// ...
	
	} // namespace
		
	void init()
	{
		// ...
		
		bleKeyboard.begin();
	}


API documentation
-----------------
The BleKeyboard interface is almost identical to the Keyboard Interface, so you can use documentation right here:
https://www.arduino.cc/reference/en/language/functions/usb/keyboard/

In addition to that you can send media keys (which is not possible with the USB keyboard library). Supported are the following:

 - KEY_MEDIA_NEXT_TRACK
 - KEY_MEDIA_PREVIOUS_TRACK
 - KEY_MEDIA_STOP
 - KEY_MEDIA_PLAY_PAUSE
 - KEY_MEDIA_MUTE
 - KEY_MEDIA_VOLUME_UP
 - KEY_MEDIA_VOLUME_DOWN
 - KEY_MEDIA_WWW_HOME
 - KEY_MEDIA_LOCAL_MACHINE_BROWSER // Opens "My Computer" on Windows
 - KEY_MEDIA_CALCULATOR
 - KEY_MEDIA_WWW_BOOKMARKS
 - KEY_MEDIA_WWW_SEARCH
 - KEY_MEDIA_WWW_STOP
 - KEY_MEDIA_WWW_BACK
 - KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION // Media Selection
 - KEY_MEDIA_EMAIL_READER

There is also Bluetooth specific information that you can set (optional):
Instead of ``BleKeyboard bleKeyboard;`` you can do ``BleKeyboard bleKeyboard("Bluetooth Device Name", "Bluetooth Device Manufacturer", 100);``. (Max length is 15 characters, anything beyond that will be truncated.)  
The third parameter is the initial battery level of your device. To adjust the battery level later on you can simply call e.g.  ``bleKeyboard.setBatteryLevel(50)`` (set battery level to 50%).  
By default the battery level will be set to 100%, the device name will be `ESP32 Bluetooth Keyboard` and the manufacturer will be `Espressif`.  
There is also a ``setDelay`` method to set a delay between each key event. E.g. ``bleKeyboard.setDelay(10)`` (10 milliseconds). The default is `8`.  
This feature is meant to compensate for some applications and devices that can't handle fast input and will skip letters if too many keys are sent in a small time frame.  
