UDP Server mDNS
===============

Instructions
------------

The multicast Domain Name System (mDNS) resolves host names to IP addresses
within small networks that do not include a local name server.

More info on mDNS can be found at https://en.wikipedia.org/wiki/Multicast_DNS

mDNS has two parts:

1. Advertise
2. Listen

This example just does Advertise.

Listen implementation is still on going work and will be basically porting
http://gkaindl.com/software/arduino-ethernet/bonjour to "Sming".

In short this code will advertise other machines about its ipaddress.

But you can not convert other mDNS advertiser's host name to ipaddress. (this is work of Listening)

How to use mDNS:

1. ADD your WIFI_SSID / Password
2. Flash the Complied code to ESP8266
3. According to OS of your PC / phone

   Mac OS (iphone/ipad/ mac)
      in Safari browser type "http://test.local/" to open a sample webpage.

   Windows
      You need Bonjour Service running. If you do not have it Install it from
      http://download.cnet.com/Bonjour-for-Windows/3000-18507_4-93550.html.

      After installing in IE or Chrome or other browser type
      "http://test.local/" to open a sample webpage.

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
