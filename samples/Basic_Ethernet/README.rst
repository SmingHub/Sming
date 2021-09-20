Basic Ethernet
==============

Demonstrates creating an ethernet connection.

Currently only supported for ESP32 using C++ wrappers around the ESP-IDF implementation.

See https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/network/esp_eth.html.


Embedded MAC

   The standard ESP32 contains an embedded ethernet MAC but requires an external PHY.
   This demonstration uses a commonly available LAN8270 module.

   See `Components/Network/Arch/Esp32/include/Platform/EmbeddedEthernet.h` for connections.

   .. note:

      The MAC/PHY interface uses high-speed signalling so connections must be solid.
      The connection may *appear* to work but fail, for example, to obtain network address.
      If this happens, check connections.

      In the current configuration a 50MHz synchronisation clock is received from the PHY on GPIO0.
      Not all ESP32 development boards have this pin available!


SPI ethernet

   SPI connections depend on the device variant being used.
   See `Sming/Libraries/HardwareSPI/src/Arch/Esp32/Controller.cpp` for details.

   See `Sming/Components/Network/Arch/Esp32/Network/W5500.cpp` for additional pin connections
   for W5500-based devices.

