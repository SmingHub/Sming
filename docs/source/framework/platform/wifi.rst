WiFi
====

Build variables
---------------

.. envvar:: ENABLE_WPS

   Set to 1 to enable WiFi Protected Setup (WPS).

   WPS is not enabled by default to preserve resources, and because there may be security implications which you should consider carefully.

.. envvar:: ENABLE_SMART_CONFIG

   Set to 1 to enable WiFi Smart Configuration API.

   SmartConfig requires extra libraries and :envvar:`ENABLE_ESPCONN`.

   See :sample:`Basic_SmartConfig` sample application.

If you want to provide a default SSID and Password for connection to your default Access Point, you can do this:

::

   make WIFI_SSID=MyAccessPoint WIFI_PWD=secret

These are provided as #defined symbols for your application to use. See :sample:`Basic_WiFi` for a simple example,
or :sample:`MeteoControl` for a more flexible solution using configuration files.

.. envvar:: WIFI_SSID

   SSID identifying default Access Point to connect to. By default, this is undefined.


.. envvar:: WIFI_PWD

   Password for the :envvar:`WIFI_SSID` Access Point, if required. If the AP is open then
   leave this undefined.


API Documentation
-----------------

.. toctree::
   :maxdepth: 1

   station
   accesspoint
   wifi-events
   wifi-sniffer
