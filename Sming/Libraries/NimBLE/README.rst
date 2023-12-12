ESP32 NimBLE
============

.. highlight:: c++

Introduction
------------
NimBLE is a completely open source Bluetooth Low Energy (BLE) stack produced by `Apache <https://github.com/apache/mynewt-nimble>`_.  
It is more suited to resource constrained devices than bluedroid and has now been ported to the ESP32 by Espressif.

Using
-----

1. Add ``COMPONENT_DEPENDS += NimBLE`` to your application component.mk file.
2. Add these lines to your application::

	#include <NimBLEDevice.h>
		
	void init()
	{
		// ...
		
		BLEDevice::init("");
	}
