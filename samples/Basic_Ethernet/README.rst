Basic Ethernet
==============

Demonstrates creating an ethernet connection.

Currently only supported for ESP32 with internal MAC.
An external PHY is required. This demonstration uses a commonly available LAN8270 module.

.. note:

   The MAC/PHY interface uses high-speed signalling so connections must be solid.
   The connection may *appear* to work but fail, for example, to obtain network address.
   If this happens, check connections.

   In the current configuration a 50MHz synchronisation clock is received from the PHY on GPIO0.
   Not all ESP32 development boards have this pin available!
