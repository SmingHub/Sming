Hosted RCP Server over TCP
==========================

Overview
--------
This application creates a RPC server that will communicate over TCP. You can either start an Access Point from the controller
or connect the application to an existing WIFI Access point. The latter can be compiled using the following command::

    make SMING_ARCH=Esp8266 CONNECT_TO_WIFI=1 WIFI_SSID="MySSID" WIFI_PWD="Secr3tP4Ssw0rd"

Configuration
-------------

.. envvar:: CONNECT_TO_WIFI

   Default: 0 (disabled)

   If set to 1 the application will try to connect to a WIFI access point. Make sure to provide also the WIFI_SSID and WIFI_PWD values.

   If set to 0 the application will start an access point to which the Host application can connect.

.. envvar:: WIFI_SSID

   Default: PleaseEnterSSID

   WIFI Access Point name. If you have enabled CONNECT_TO_WIFI then make sure to set also WIFI_SSID and WIFI_PWD values.

.. envvar:: WIFI_PWD

   Default: PleaseEnterPass

   WIFI Access Point password. If you have enabled CONNECT_TO_WIFI then make sure to set also WIFI_SSID and WIFI_PWD values.
