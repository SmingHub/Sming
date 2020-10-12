mDNS: Multicast Domain Name System
==================================

.. highlight:: c++

https://en.wikipedia.org/wiki/Multicast_DNS

Sming provides the :cpp:class:`mDNS::Responder` class to allow applications
to advertise themselves on the local network.

Issuing discovery requests is not currently supported.


Using
-----

1. Add ``COMPONENT_DEPENDS += mdns`` to your application componenent.mk file.
2. Add these lines to your application::

   #include <Mdns/Responder.h>

   static mDNS::Responder responder;
   
   // Call when IP address has been obtained
   void startmDNS()
   {
      responder.begin(F("myhostname");
   }

This will advertise the device as ``myhostname.local``.

To provide a custom service, implement a :cpp:class:`mDNS::Service` class
and call :cpp:func:`mDNS::Responder::addService`.

See the :sample:`UdpServer_mDNS` sample application.


Testing
-------

For linux, you can use `avahi <https://wiki.archlinux.org/index.php/Avahi>`__
to perform mDNS queries and confirm output is as expected:

.. code-block:: bash

   sudo apt install avahi
   avahi-browse --all -r


API Documentation
-----------------

.. doxygennamespace:: mDNS
