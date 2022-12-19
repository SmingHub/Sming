Switch Joycon
=============

Introduction
------------
This sample turns the ESP32 into a Switch Joycon (Bluetooth LE gamepad) that presses buttons and moves axis

Possible buttons are 0 through to 15.

Possible HAT switch position values are:
Centered, Up, UpRight, Right, DownRight, Down, DownLeft, Left, UpLeft.


Testing
-------

You can use one of the following applications on your PC to test and see all buttons that were clicked.

On Linux install ``jstest-gtk`` to test the ESP32 gamepad. Under Ubuntu this can be done by typing the following command::

	sudo apt install jstest-gtk
	
On Windows use this `Windows test application <http://www.planetpointy.co.uk/joystick-test-application/>`__.