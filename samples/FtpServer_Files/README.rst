FTP Server Files
================

.. highlight:: bash

This example sets up a simple FTP server with a couple of files stored in SPIFFS.

You'll need to have WiFi configured. You can set this information when building like this::

   make WIFI_SSID=ssid WIFI_PWD=password

substituting your actual Access Point details for *ssid* and *password*.

After flashing::

   make flashapp

You should be able to connect using an FTP client:

   ftp ipaddress

