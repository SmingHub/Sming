SSL: Secure Sockets Layer
=========================

https://en.m.wikipedia.org/wiki/Transport_Layer_Security

Sming supports multiple SSL implementations, currently with adapters for:

-  `axTLS <http://axtls.sourceforge.net/>`__
-  `BearSSL <https://www.bearssl.org/>`__

If you want to use SSL then take a look at the :sample:`Basic_Ssl` sample for creating SSL clients,
and :sample:`HttpServer_ConfigNetwork` for SSL servers.

Certificates
------------

If you need an X509 certificate and private key::

   make generate-cert

This will use openssl to generate binary certificate and key information in ``out/ssl``
and create ``include/ssl/cert.h`` and ``include/ssl/private_key.h``.
See :sample:`MqttClient_Hello` for example code.

An alternative to generating header files is to use the binary certificate files directly.
See :sample:`Basic_AWS` for an example of this approach.


Configuration Variables
-----------------------

.. envvar:: ENABLE_SSL

   -  0 (default): SSL requires lots of RAM and some intensive processing, so to conserve resources it is disabled by default.
   -  1: to enable the default SSL adapter. At the moment that is Axtls.
   -  Axtls: to enable SSL support using the :component:`axtls-8266` component.
   -  Bearssl: to enable SSL support using the :component:`bearssl-esp8266` component.


API Documentation
-----------------

.. toctree::
   :maxdepth: 1

   upgrade
   comparison
   session
   ciphersuites
   certificates
   adapter
