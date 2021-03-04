mDNS: Multicast Domain Name System
==================================

.. highlight:: c++

https://en.wikipedia.org/wiki/Multicast_DNS


Responder
---------

Sming provides the :cpp:class:`mDNS::Responder` class to allow applications
to advertise themselves on the local network.

To use:

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


Discovery
---------

This library also provides support for device discovery using a separate set of classes,
based on the :cpp:class:`mDNS::Server`.
See :sample:`Basic_Mdns` for an example.

.. note::

   The mDNS Server and Responder cannot currently be used together.


Testing
-------

For linux, you can use `avahi <https://wiki.archlinux.org/index.php/Avahi>`__
to perform mDNS queries and confirm output is as expected:

.. code-block:: bash

   sudo apt install avahi
   avahi-browse --all -r


References
----------

-  Multicast DNS RFC6762 https://tools.ietf.org/html/rfc6762
-  Zero-configuration networking (DNS-SD) https://en.wikipedia.org/wiki/Zero-configuration_networking
-  DNS-Based Service Discovery https://tools.ietf.org/html/rfc6763
-  DNS record types https://en.wikipedia.org/wiki/List_of_DNS_record_types
-  Domain Names: Implementation and Specification https://tools.ietf.org/html/rfc1035


API Documentation
-----------------

.. doxygennamespace:: mDNS
