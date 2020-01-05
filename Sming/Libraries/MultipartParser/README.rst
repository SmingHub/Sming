Multipart Parser
================

Component to manage processing of multipart form data according to 
`RFC7578 <https://tools.ietf.org/html/rfc7578>`_, mostly used to support file
uploads via HTTP/POST.

Usage
-----

While setting up your web server, register the body parser provided by the
library:

.. code-block:: c++

   #include <MultipartParser.h>

   HttpServer server;
   ...

   server.setBodyParser(MIME_FORM_MULTIPART, formMultipartParser);

Now add a :cpp:class:`HttpMultipartResource` for the path to receive the multipart data:

.. code-block:: c++
   
   void fileUploadMapper(HttpFiles& files)
   {
       files["file"] = <writable_stream_to_process_file>;
   }

   int onUpload(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
   {
       // `file` points to the stream from `fileUploadMapper`.
       auto* file = request.files["file"];
       
       // TODO: use methods of `file` to check if data was processed successfully

       // TODO: setup HTTP response
       
       return 0;
   }

   ...

   server.paths.set("/upload", new HttpMultipartResource(fileUploadMapper, onUpload));


See :sample:`HttpServer_FirmwareUpload` for further details.

Upgrade Notes
-------------

The functionality provided by this lirbary was previously controlled by the config option
`ENABLE_HTTP_SERVER_MULTIPART`.

To upgrade, you have to replace:: 
   
   ENABLE_HTTP_SERVER_MULTIPART := 1

by::
   
   ARDUINO_LIBRARIES += MultipartParser

in your `component.mk`. In addition, body parser registration must now be done explicitly by 
application (see above).

API Documentation
-----------------

.. toctree::
   :maxdepth: 1

   api
