Bluetooth Gamepad
=================

Introduction
------------
This sample turns the ESP32 into a Bluetooth LE gamepad that presses buttons and moves axis

Possible buttons are: BUTTON_1 through to BUTTON_16
(16 buttons supported by default. Library can be configured to support up to 128)

Possible DPAD/HAT switch position values are:
DPAD_CENTERED, DPAD_UP, DPAD_UP_RIGHT, DPAD_RIGHT, DPAD_DOWN_RIGHT, DPAD_DOWN, DPAD_DOWN_LEFT, DPAD_LEFT, DPAD_UP_LEFT
(or HAT_CENTERED, HAT_UP etc)

bleGamepad.setAxes takes the following int16_t parameters for the Left/Right Thumb X/Y, Left/Right Triggers plus slider1 and slider2, and hat switch position as above:
(Left Thumb X, Left Thumb Y, Right Thumb X, Right Thumb Y, Left Trigger, Right Trigger, Hat switch position ^ (1 hat switch (dpad) supported by default. Library can be configured to support up to 4)

Library can also be configured to support up to 5 simulation controls (can be set with setSimulationControls)
(rudder, throttle, accelerator, brake, steering), but they are not enabled by default.


Testing
-------

You can use one of the following applications on your PC to test and see all buttons that were clicked.

On Linux install ``jstest-gtk`` to test the ESP32 gamepad. Under Ubuntu this can be done by typing the following command::

	sudo apt install jstest-gtk
	
On Windows use this `Windows test application <http://www.planetpointy.co.uk/joystick-test-application/>`__.
 