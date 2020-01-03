HTTP: HyperText Transfer Protocol
=================================

https://en.m.wikipedia.org/wiki/Hypertext_Transfer_Protocol

Build Variables
---------------


.. envvar:: HTTP_SERVER_EXPOSE_NAME

   Default: 1 (enabled)

   Adds "HttpServer/Sming" to the SERVER field in response headers.
   If disabled, the SERVER field is omitted from all responses.


.. envvar:: HTTP_SERVER_EXPOSE_VERSION

   Default: 0 (disabled)

   Adds the current Sming build version to the SERVER field in response headers.
   For example, "Sming/4.0.0-rc2".

   Requires HTTP_SERVER_EXPOSE_NAME to be enabled.


.. envvar:: HTTP_SERVER_EXPOSE_DATE

   Default: 0 (disabled)

   Sets the DATE field in response headers.


.. envvar:: ENABLE_HTTP_SERVER_MULTIPART

   Default: 0 (disabled)

   Set to 1 to enable form upload support on the server.

   See :sample:`HttpServer_FirmwareUpload` sample application.


Support API
-----------

.. doxygennamespace:: ContentType


Client API
----------

.. doxygengroup:: httpclient
   :content-only:


Server API
----------

.. doxygengroup:: httpserver
   :content-only:
