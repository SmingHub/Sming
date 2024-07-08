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

For server applications you'll require an X509 certificate and private key.
This is typically generated using openssl, then the resulting binary files added to your project.
The :sample:`Basic_AWS` demonstrates this approach.

.. important::

   Clearly private key information must be guarded so take care not to publish this in a public repository!

Sming also has a convenience script you can use::

   make generate-cert

This generates binary certificate and key information in ``out/ssl``,
and creates ``include/ssl/cert.h`` and ``include/ssl/private_key.h``.
See :sample:`MqttClient_Hello` for example code.


Enabling SSL in HttpServer
--------------------------

.. highlight:: c++

The :cpp:func:`TcpServer::listen` method, and the child :cpp:class:`HttpServer` class,
accepts a second optional ``useSsl`` parameter.
If you look at the ``startWebServer`` function in :source:`samples/HttpServer_WebSockets/app/application.cpp`,
it can be changed to something like::

   void startWebServer()
   {
       // TODO: Make sure to set a server certificate and key
       server.listen(443, true);

And what is left is the actual setting of the server certificate::

   void startWebServer()
   {
       // Assign the certificate
       server.setSslInitHandler([](Ssl::Session& session) {
            session.keyCert.assign(serverKey, serverCert);
       });
       server.listen(443, true);

The final code can be something like::

  void startWebServer()
  {
  #ifdef ENABLE_SSL
    server.setSslInitHandler([](Ssl::Session& session) {
      session.keyCert.assign(serverKey, serverCert);
    });
    server.listen(443, true);
  #else
    server.listen(80);
  #endif
    server.paths.set("/", onIndex);
    //...

This is also demonstrated for secure MQTT in the :sample:`MqttClient_Hello` sample.


Security Considerations
=======================

Does it really make sense to use SSL for an HttpServer on an ESP8266 device?

The certificate/private key pair should make it impossible for an
external user to decrypt your traffic so that the things that you sent
are kept private, but there are some complications with this:

-  The private key will not stay private for long. The private key should be
   kept encrypted on the flash memory, to prevent casual reading.
   But even with decryption there is a high probability that someone
   will be able to disassemble your application and figure out how to
   decrypt the key.

-  Costs for certificate. Let's imagine that you have overcome the first
   issue. Then comes the second issue - if you want your users to accept
   the certificate it has to be signed by one of the trusted certificate
   authorities. And that costs money. And if you want to use a unique
   certificate/private key pair for every device than it will make
   things worse, moneywise. Note: Free SSL certificates are now available,
   for example https://letsencrypt.org/. These will expire if not kept
   up to date so adds additional complexity to your application.

-  You can handle up to 2 or maximum 3 connections. SSL needs 16K of memory to
   make the initial handshake. The memory consumption after a successful
   handshake can decrease to 4K, just for the SSL, per request. But
   realistically this means that you will end up with a server that can
   handle maximum 2 or 3 simultaneous connections before the heap memory is
   consumed and has to be released.

Therefore, in our humble opinion, it would be better to rely on the WIFI security that your Access
Point (AP) provides and make this AP accessible only for your IoT devices.



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
