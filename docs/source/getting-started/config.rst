Configuring your device
-----------------------

.. highlight:: bash

You may need to configure your project to support the specific device being programmed:

* :envvar:`COM_PORT` If you haven't set this already, it will need to match the port you're using to talk to the Esp8266.
* :envvar:`COM_SPEED_ESPTOOL` The default value should work fine but you can usually set a much faster speed.

You can set these initially on the command line, like this::

   make SMING_ARCH=Esp8266 COM_PORT=/dev/ttyUSB3 COM_SPEED_ESPTOOL=921600

(For Windows or :doc:`windows/wsl` expect to use COM2, COM3, etc.)

Once you're happy with the settings, you can add them to your project's ``component.mk`` file.
You may need to do this to reset the cached values::

   make config-clean

The current set of configuration variables can be seen thus::

   make list-config

Other hardware-specific settings are stored in the hardware configuration file.
You can examine the current configuration like this::

   make hwconfig

The standard config should work with all ESP8266 variants.
If you want to use SPIFFS then you should add this line to your component.mk file::

   HWCONFIG = spiffs

This expects your device to have at least 4MBytes of flash.

* See :doc:`/_inc/Sming/building` for further details about configuring your project.
* See :doc:`/features` for configuring Sming options.
