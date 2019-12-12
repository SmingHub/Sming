SSL: Secure Sockets Layer
=========================

https://en.m.wikipedia.org/wiki/Transport_Layer_Security

Configuration Variables
-----------------------

.. envvar:: ENABLE_SSL

   Default: undefined (disabled)

   SSL requires lots of RAM and some intensive processing, so to conserve resources it is disabled by default.
   If you want to enable it then take a look at the :sample:`Basic_Ssl` sample.

   Set to 1 to enable SSL support using the :component:`axtls-8266` Component.


API Documentation
-----------------

.. doxygengroup:: ssl
   :content-only:
