HttpServer Firmware Upload
=========================

Introduction
------------

The HTTP server coming with Sming is quite powerful but it is limited
from the available resources of the underlining hardware (your favorite
ESP8266 microcontroller).

This sample demonstrates how to enable file upload of the HTTP server.
On a normal computer the file uploads are usually using
temporary space on the hard disk or in memory to store the incoming data.

On an embedded device that is a luxury that we can hardly afford.
In this sample we demonstrate how to define which file upload fields
should be stored and what (file) streams are responsible for storing the data.
If a field is not specified then its content will be discarded.