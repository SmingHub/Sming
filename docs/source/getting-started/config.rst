Configuration
=============

.. highlight:: bash

Environment variables
---------------------

Certain environment variables should be set globally so that command prompts
and integrated development environments (IDEs) work correctly.

You can find a list of these in :source:`Tools/export.sh`.

For Linux and WSL2, append :envvar:`SMING_HOME` to your ``~/.bashrc`` file::

   # All architectures
   export SMING_HOME=/opt/sming/Sming
   
:envvar:`SMING_HOME` is the only mandatory environmental variable.

If you want to change the location where the toolchain will be downloaded and installed you can append the values below::
	
   # Specifies a common toolchains directory
   export SMING_TOOLCHAINS=/opt
   
The :envvar:`SMING_TOOLCHAINS` is optional. As are the ones below. You can append them to your ``~/.bashrc`` file only
if you need to change the location of the installed toolchains::
  
   # Esp8266
   export ESP_HOME=/opt/esp-quick-toolchain

   # Esp32
   export IDF_PATH=/opt/esp-idf
   export IDF_TOOLS_PATH=/opt/esp32

   # Rp2040
   export PICO_TOOLCHAIN_PATH=/opt/rp2040

Another approach is to place these in a separate file, then add `source ~/.smingrc`.

For Windows, you can use the provided script::

.. code-block:: batch

   %SMING_HOME%\..\Tools\export -persist

Note that project-specific settings should NOT be configured globally.
Please use the project's ``component.mk`` file for that purpose.


Configuring your device
-----------------------

You may need to configure your project to support the specific device being programmed:

* :envvar:`COM_PORT` If you haven't set this already, it will need to match the port you're using to talk to the target device.
* :envvar:`COM_SPEED_ESPTOOL` The default value should work fine but you can usually set a much faster speed.

You can change these on the command line::

   make SMING_ARCH=Esp8266 COM_PORT=/dev/ttyUSB3 COM_SPEED_ESPTOOL=921600

For Windows or :doc:`windows/wsl` expect to use COM2, COM3, etc.

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
