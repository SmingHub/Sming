UDP Server mDNS
===============

Instructions
------------

The multicast Domain Name System (mDNS) resolves host names to IP addresses
within small networks that do not include a local name server.

More info on mDNS can be found at https://en.wikipedia.org/wiki/Multicast_DNS

mDNS has two parts:

1. Advertise
2. Discover

This example just does Advertise. See :sample:`Basic_Mdns` for discovery example.

In short this code will advertise other machines about its ipaddress.

How to use mDNS:

1. ADD your WIFI_SSID / WIFI_PWD
2. Flash the Complied code to your ESP8266/ESP32 device
3. Open a web browser and go to "http://UDP_Server.local/" to open a sample webpage.

You should also be able to ping using the advertised name::

   ping UDP_Server.local

Linux

   You need to install Avahi mDNS/DNS-SD daemon.

      In your browser type "http://test.local/" to open a sample webpage.

Android

   You need to install ZeroConf Browser or Bonjour Browser.

   In those app you would be able to see IP address of your ESP module.

   In android Chrome "http://test.local/" does not translate to IP address,
   so android Chrome is not supporting mDNS.
   
   But you can make your own app using Network Service Discovery.
   See http://developer.android.com/training/connect-devices-wirelessly/nsd.html for details.
