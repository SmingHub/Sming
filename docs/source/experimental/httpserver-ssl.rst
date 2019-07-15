****************************
How to use SSL in HttpServer
****************************

I will try to split my answer into two main areas

* At the moment the HttpServer (also with Websockets) supports setting
  TLS/SSL
* but the question is how much sense is that making?

Enabling SSL in HttpServer
==========================

The ``listen`` method in the HttpServer class accepts a second optional
parameter. If you look at the original code:
https://github.com/SmingHub/Sming/blob/develop/samples/HttpServer_WebSockets/app/application.cpp#L95-L99

That can be changed to something like:

.. code-block:: c++

   void startWebServer()
   {
           // TODO: Make sure to set a server certificate and key
       server.listen(443, true);

And what is left is the actual setting of the server certificate:

.. code-block:: c++

   void startWebServer()
   {
           SSLKeyCertPair clientCertKey;
           // TODO: Make sure to set a server certificate and key
           server.setSslCertKey(clientCertKey);
       server.listen(443, true);

And the final code can be something like:

.. code-block:: c++

   void startWebServer()
   {

   #ifdef ENABLE_SSL
   #include "ssl/server_cert.h"
   #include "ssl/server_private_key.h"
       SSLKeyCertPair clientCertKey;
       clientCertKey.certificate = new uint8_t[default_certificate_len];
       memcpy(clientCertKey.certificate, default_certificate,
              default_certificate_len);
       clientCertKey.certificateLength = default_certificate_len;
       clientCertKey.key = new uint8_t[default_private_key_len];
       memcpy(clientCertKey.key, default_private_key, default_private_key_len);
       clientCertKey.keyLength = default_private_key_len;
       clientCertKey.keyPassword = NULL;

       server.setServerKeyCert(clientCertKey);

      server.listen(443, true);
   #else
       server.listen(80, false);
   #endif

Does it really make sense to use SSL for an HttpServer on an ESP8266 device?
============================================================================

The certificate/private key pair should make it impossible for an
external user do decrypt your traffic so that the things that you sent
are kept private, but there are some complications with this: - The
private key will not stay private for long: The private key should be
kept encrypted on the flash memory. Otherwise it can be easily read from
it. But even with decryption there is a high probability that someone
will be able to disassemble your application and figure out how to
decrypt the key.

-  Costs for certificate: Let’s imagine that you have overcome the first
   issue. Then comes the second issue - if you want your users to accept
   the certificate it has to be signed by one of the trusted certificate
   authorities. And that costs money. And if you want to use a unique
   certificate/private key pair for every device than it will make
   things worse, moneywise.

-  You can handle up to 2 max 3 connections. SSL needs 16K of memory to
   make the initial handshake. The memory consumption after a successful
   handshake can decrease to 4K, just for the SSL, per request. But
   realistically this means that you will end up with a server that can
   handle max 2 or 3 simultaneous connections before the heap memory is
   consumed and has to be released.

So IMHO it would be better to rely on the WIFI security that your Access
Point (AP) provides and make this AP accessible only for your IoT
devices.
