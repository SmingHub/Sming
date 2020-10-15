SSDP: Simple Service Discovery Protocol
=======================================

.. highlight:: c++

https://en.wikipedia.org/wiki/Simple_Service_Discovery_Protocol

Sming provides the :cpp:class:`Ssdp` for advertisement and discovery of network services and presence information in the local network.

Using
-----

1. Add ``COMPONENT_DEPENDS += ssdp`` to your application componenent.mk file.
2. Add these lines to your application::

   #include <Network/Ssdp.h>

   Ssdp* ssdp = nullptr;

   // Call when IP address has been obtained
   void startSsdp()
   {
      ssdp = new Ssdp();
      // Notice: Before connecting you can set different settings as for example friendlyName of the device, unique id and so on
      //         See Ssdp::Settings documentation for details.

      Ssdp::Settings settings;
      settings["friendlyName"] = "Sming Tea Pot";
      settings["deviceType"] = "upnp:tea-pot"; // This device is a tea pot
      settings["UDN"] = "uuid:1231313131::upnp:tea-pot"; // This is the unique id of the device.
      ssdp->setSettings(settings);

      ssdp->connect();
      ssdp->search("urn:dial-multiscreen-org:service:dial:1"); // The device is searching for monitors.

   }

See the :sample:`Ssdp` sample application for details.

Testing
-------

Under Ubuntu Linux you can install `gupnp-tools`::

   sudo apt install gupnp-tools

 And then search for your device in the local network using the following command::

   gssdp-discover --target=upnp:tea-pot


API Documentation
-----------------

.. doxygennamespace:: Ssdp
