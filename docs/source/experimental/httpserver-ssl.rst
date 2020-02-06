****************************
Enabling SSL in HttpServer
****************************

.. highlight:: c++

* At the moment any TCP based server in Sming can use TLS/SSL.
* That applies to HttpServer (also with Websockets).
* But make sure to read the security considerations and limitations.

Enabling SSL in HttpServer
==========================

The ``listen`` method in the TcpServer, and the child HttpServer class, accepts a second optional
parameter. If you look at the original code:
:source:`samples/HttpServer_WebSockets/app/application.cpp#L95-L99`.

That can be changed to something like::

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

-  Costs for certificate. Let’s imagine that you have overcome the first
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
