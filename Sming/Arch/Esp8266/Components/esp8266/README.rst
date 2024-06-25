Esp8266 Core Component
======================

.. highlight:: bash

Contains startup code, crash handling and additional Esp8266-specific
support code.

Sming uses libraries from the ESP8266 NON-OS SDK version 3, imported as a submodule.
The header and linker files are provided by this Component.


Configuration variables
-----------------------

.. envvar:: ENABLE_CUSTOM_PHY

   Default: undefined (off)

   The ``phy_init`` partition contains data which the ESP8266 SDK uses to initialise WiFi hardware at startup.

   You may want to change settings for a certain ROM on the device without changing it for all ROMs on the device.
   To do this, build with ``ENABLE_CUSTOM_PHY=1`` and add code to your application::

      #include <esp_phy.h>

      void customPhyInit(PhyInitData data)
      {
         // Use methods of `data` to modify as required
      	data.set_vdd33_const(0xff);
      }

   See :cpp:struct:`PhyInitData` for further details.


.. envvar:: FLASH_INIT_DATA

   Read-only. This is the path to the default PHY data written to the ``phy_init`` partition.
   It is provided by the SDK.


.. envvar:: FLASH_INIT_DATA_VCC

   Read-only. This is the path to a modified version of the default PHY data selected
   by the ``vdd`` hardware configuration option. See :doc:`/information/tips-n-tricks`.

   The modification is equivalent to calling :cpp:func:`PhyInitData::set_vdd33_const` with ``0xff``.


API reference
-------------

.. doxygenstruct:: PhyInitData
    :members:
