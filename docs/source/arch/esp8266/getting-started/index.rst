Getting Started: ESP8266
========================

Development Environment
-----------------------

.. toctree::
   :glob:
   :maxdepth: 1

   *

Configuring your Esp8266 device
-------------------------------

.. highlight:: bash

You may need to configure your project to support the specific device being programmed.

.. note::

   Arduino has a system which allows these values to be set based on a *board*
   selection. At present Sming does not have such a system so this must be done
   manually.

Here are a few important ones:
 
* :envvar:`SPI_MODE` Change this if your device fails to program or run
* :envvar:`SPI_SIZE` If using a device with reduced memory you may need to change this value. Note also that samples using :component:`spiffs` may not work.
* :envvar:`COM_PORT` If you haven't set this already, it will need to match the port you're using to talk to the Esp8266.
* :envvar:`COM_SPEED_ESPTOOL` The default value should work fine but you can usually set a much faster speed.

You can set these initially on the command line, like this:

::

   make SMING_ARCH=Esp8266 SPI_MODE=dio SPI_SIZE=4M COM_SPEED_ESPTOOL=921600

Once you're happy with the settings, you can add them to your ``project.mk`` file.

You can list the current set of configuration variables like this:

::

   make list-config

If you want to reset back to default values, do this:

::

   make config-clean


* See :doc:`/_inc/Sming/building` for further details about configuring your project.
* See :doc:`/features` for configuring Sming options.
