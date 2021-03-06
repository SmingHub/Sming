Basic Web Skeleton
==================

Introduction
------------

Basic application that can be used as a start point for some useful App.

Features:

-  can setup wifi ssid and wifi password for STA (wifi client) mode
   either from own AP or as connected to some wifi network
-  demonstrate new way of catching wif-events with WifiEvents class
-  if preconfigured wifi network is unreachable start AP named TyTherm
   with hardcoded password (see source)
-  can enable/disable STA (wifi client) mode
-  own AP autodisable after successful connection to preconfigured wifi
   network
-  form population and sending is done with json+ajax
-  demonstrate usage of getting raw http request body to be processed as
   json
-  demonstrate how to fill html template on client side with more
   flexible than Smings Templating - JavaScript

App called TyTherm because it is base for TinY TermOmeter :)

FlashString
-----------

This sample also demonstrates how to use FlashString maps as an alternative
to using SPIFFS for serving files.

To test this out, build the application without a filesystem image:

.. code-block:: bash

   make HWCONFIG=standard ENABLE_FLASHSTRING_MAP=1

See *webserver.cpp* for the details.
