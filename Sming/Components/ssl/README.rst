SSL: Secure Sockets Layer
=========================

https://en.m.wikipedia.org/wiki/Transport_Layer_Security

Sming supports multiple SSL implementations, currently with adapters for:

-  `axTLS <http://axtls.sourceforge.net/>`__
-  `BearSSL <https://www.bearssl.org/>`__

If you want to use SSL then take a look at the :sample:`Basic_Ssl` for creating SSL clients and :sample:`HttpServer_ConfigNetwork`
for SSL servers.


Configuration Variables
-----------------------

.. envvar:: ENABLE_SSL

   - 0 (default): SSL requires lots of RAM and some intensive processing, so to conserve resources it is disabled by default.
   - 1: to enable the default SSL implementation. At the moment that is Axtls.
   - Axtls: to enable SSL support using the :component:`axtls-8266` component.
   - Bearssl: to enable SSL support using the :component:`bearssl-esp8266` component.

.. toctree::
   :glob:
   :maxdepth: 1

   *
