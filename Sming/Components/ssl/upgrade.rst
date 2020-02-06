SSL: Upgrading
==============

.. highlight:: c++

Introduction
------------

Sming v4.1 introduced some major changes in the SSL architecture to support multiple adapters.

The default adapter is still based on axTLS, and it can be enabled in your application by providing
the :envvar:`ENABLE_SSL` directive either in your component.mk file or during compilation.

Migration
---------

The access to the SSL connection from the TcpConnection is simplified and fetching information about SSL certificate and session id is easier
than before.

The old code was looking like this::

   SSL* ssl = connection.getSsl();
   if(ssl) {
     const char* common_name = ssl_get_cert_dn(ssl, SSL_X509_CERT_COMMON_NAME);
     if(common_name) {
       debugf("Common Name:\t\t\t%s\n", common_name);
     }
     displayCipher(ssl);
     displaySessionId(ssl);
   }


Now it should be migrated to the following shorter version::

   auto ssl = connection.getSsl();
   if(ssl != nullptr) {
     ssl->printTo(Serial);
   }


SSL initialisation in TCP clients or servers is done using an :cpp:type:`Ssl::Session::InitDelegate` callback.

Old code looking like this::

   MqttClient* getMqttClient()
   {
     if(mqtt == nullptr) {
       mqtt = new MqttClient();
       mqtt->addSslOptions(SSL_SERVER_VERIFY_LATER); // << this is where we were setting SSL options
       Url url;
   
Has to be migrated to the following code::

   void sslInit(Ssl::Session& session)
   {
     session.options.verifyLater = true;
   }
   
   MqttClient* getMqttClient()
   {
     if(mqtt == nullptr) {
       mqtt = new MqttClient();
       mqtt->setSslInitHandler(sslInit); // << this is where the sslInit callback is set
       Url url;


It is possible to create an SSL enabled server. The excerpt below demonstrates this and it is part of the :sample:`HttpServer_ConfigNetwork` sample.
Pay attention to the :doc:`security considerations </experimental/httpserver-ssl>` and limitations using this on a microcontroller with limited RAM::

   void startWebServer()
   {
   #ifdef ENABLE_SSL
     server.setSslInitHandler([](Ssl::Session& session) {
       debug_i("SSL Init handler: setting server keyCert");
       session.keyCert.assign(serverKey, serverCert);
     });
     server.listen(443, true);
   #else
     server.listen(80);
   #endif
     server.paths.set("/", onIndex);
     server.paths.set("/ipconfig", onIpConfig);
     server.paths.set("/ajax/get-networks", onAjaxNetworkList);
     server.paths.set("/ajax/connect", onAjaxConnect);
     server.paths.setDefault(onFile);
   }


Setting client certificates, ssl options and pinning for a HttpRequest is done using onSslInit callback.
If you look at the :sample:`Basic_Ssl` sample you will see that the old way of setting them was as shown below::

   HttpRequest* request = new HttpRequest(F("https://www.grc.com/fingerprints.htm"));
   request->setSslOptions(SSL_SERVER_VERIFY_LATER);
   request->pinCertificate(fingerprints);
   
The new one is using the following sequence of commands::

   auto request = new HttpRequest(F("https://www.grc.com/fingerprints.htm"));
   request->onSslInit(grcSslInit);


A sample callback is given below. In the callback the developer
has access to the current SSL session and HTTP request and can modify them accordingly::

   void grcSslInit(Ssl::Session& session, HttpRequest& request)
   {
     static const Ssl::Fingerprint::Cert::Sha1 fingerprint PROGMEM = {  ... };

     session.validators.pin(fingerprint);

     // We're using validators, so don't attempt to validate full certificate
     session.options.verifyLater = true;

     session.fragmentSize = Ssl::eSEFS_16K;
   }

Note also that the ``Fingerprints`` class has been removed.
Instead, we use methods of :cpp:class:`session.validators <Ssl::ValidatorList>` to:

-  Pin fingerprints;
-  Add one or more custom callback validators;
-  Implement custom validators by inheriting from :cpp:class:`Ssl::Validator`.


Cryptographic support
~~~~~~~~~~~~~~~~~~~~~

Some basic class-based cryptographic support is provided via the :component:`crypto` library,
organised within the ``Crypto`` namespace.

This is primarily for use with the SSL interface but does not require SSL to be enabled.

The cryptographic 'C' libraries themselves may be used directly by your application, regardless
of which SSL adapter is in use, or even if SSL is disabled.

For example the following old code is using axTLS cryptographic functions::

   char* loadPsk(int* keylen)
   {
     SHA1_CTX sha_ctx;
     // ...
     SHA1_Init(&sha_ctx);
     SHA1_Update(&sha_ctx, (uint8_t*)buffer, strlen(buffer));
     SHA1_Final(digest, &sha_ctx);
   
For this code to work you should include the following header::

   #include <axtls-8266/crypto/crypto.h>


And also make sure that your application `component.mk` file has the following line::

   COMPONENT_DEPENDS += axtls-8266


SSL namespace
~~~~~~~~~~~~~

All SSL related classes and types are organized in a separate namespace called ``Ssl``.
For example you should use ``Ssl::KeyCertPair`` instead of ``SslKeyCertPair`` and ``Ssl::Fingerprints`` instead of ``SslFingerprints``.
