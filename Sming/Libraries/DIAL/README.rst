DIscovery And Launch (DIAL)
===========================

.. highlight:: c++

Introduction
------------

DIAL—for DIscovery And Launch—is a simple protocol that second-screen devices can use to discover and launch apps on first-screen devices.
For example, your can stream a video from your embedded device on your connected TV.

Using
-----

1. Add ``COMPONENT_DEPENDS += DIAL`` to your application componenent.mk file.
2. Add these lines to your application::

   #include <Dial/Client.h>

   static Dial::Client client;

   void onConnected(Dial::Client& client, const XML::Document& doc, const HttpHeaders& headers)
   {
      // Get an app and do something...
   }

   // Call when IP address has been obtained
   void onIp(IpAddress ip, IpAddress mask, IpAddress gateway)
   {
      // ...

      /* The command below will use UPnP to auto-discover a DIAL enabled server */
      client.connect(onConnected);

      // ...
   }

See the :sample:`DIAL_Client` sample application.

API Documentation
-----------------

.. doxygennamespace:: DIAL
