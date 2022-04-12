Basic Blink
===========

Simple blink example to confirm that the basic build system is working with your system.

We use Timer instead of a loop because we want to allow WiFi communications to work in the background.
See :doc:`/information/multitasking`.

The LED on many development boards is connected to GPIO2, so this is the default.

If you get no response then check the documentation or schematic as your system
may differ and change the LED_PIN definition accordingly.

For example, the NodeMCU ESP-C3 kits have an RGB LED connected to GPIO 3, 4 & 5.


.. image:: blink.jpg
   :height: 192px
