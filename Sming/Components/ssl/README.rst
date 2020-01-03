SSL: Secure Sockets Layer
=========================

Provides Secure Socket Layer (SSL) support for Sming with selectable implementation.

https://en.m.wikipedia.org/wiki/Transport_Layer_Security


Configuration Variables
-----------------------

Sming supports multiple SSL implementations.

At the moment there are SSL adapters based on `axTLS <http://axtls.sourceforge.net/>`__ and `BearSSL <https://www.bearssl.org/>`__

If you want to use SSL then take a look at the :sample:`Basic_Ssl` for creating SSL clients and :sample:`HttpServer_ConfigNetwork`
for SSL servers.


.. envvar:: ENABLE_SSL

   - 0 (default): SSL requires lots of RAM and some intensive processing, so to conserve resources it is disabled by default.
   - 1: to enable the default SSL implementation. At the moment that is Axtls.
   - Axtls: to enable SSL support using the :component:`axtls-8266` component.
   - Bearssl: to enable SSL support using the :component:`bearssl-esp8266` component.

.. toctree::
   :glob:
   :maxdepth: 1

   *
