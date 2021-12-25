HttpServer Upload
=================

This example demonstrates how to upload multiple files on the server.


About HTTP server file upload
-----------------------------

The HTTP server coming with Sming is quite powerful but it is limited
by available hardware resources.

This sample demonstrates how to use the :library:`MultipartParser` library
to enable file upload of the HTTP server. On a normal computer the file uploads
are usually using temporary space on the hard disk or in memory to store the
incoming data.

On an embedded device that is a luxury that we can hardly afford.
In this sample we demonstrate how to define which file upload fields
should be recognized and what (file) streams are responsible for processing and
storing the data.
If a field is not specified then its content will be discarded.

Usage instructions
------------------

1. Configure your flash memory layout. See :ref:`hardware_config`.

2. Build the example by running::

      make

3. Connect your device via USB/Serial cable and run::

      make flash

   to install the example firmware. 

4. Point the browser to your device's IP address to open the firmware upgrade page.

5. Select the upload file and hit the "Update" button.
   
   If the file size isn't bigger than the MAX_FILE_SIZE defined in the application code 
   you should see a confirmation that the upload was successful.