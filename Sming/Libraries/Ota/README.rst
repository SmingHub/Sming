Over-The-Air(OTA) Upgrader
==========================

.. highlight:: c++

Introduction
------------

This architecture-agnostic component adds support for Over-The-Air upgrades.

Usage
-----
1. Add ``COMPONENT_DEPENDS += Ota`` to your application componenent.mk file.
2. Add these lines to your application::

      #include <Ota/Manager.h>

After that you will have access to a global ``OtaManager`` instance that can be used to manage your OTA upgrade process.

3. You can use ``OtaManager`` to get information about the bootable partitions and update them.
   The code below will display the current bootable and running partition::

      void init()
      {

          // ...
          auto partition = OtaManager.getRunningPartition();

          Serial.printf("\r\nCurrently running %s @ 0x%08x.\r\n", partition.name().c_str(), partition.address());

      }

4. If needed you can also create your own instance of the of OtaUpgrader as shown below::


      // Call when IP address has been obtained
      void onIp(IpAddress ip, IpAddress mask, IpAddress gateway)
      {
         // ...

         OtaUpgrader ota;

         auto part = ota.getNextBootPartition();

         ota.begin(part);

         // ... write all the data to the partition

         ota.end();

         // ...
      }

See the :sample:`Upgrade` sample application.

API Documentation
-----------------

.. doxygennamespace:: Ota
   :members:

