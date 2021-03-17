FTP Server Files
================

.. highlight:: bash

This example sets up an FTP server with a couple of files stored in SPIFFS.
It mounts this on top of an FWFS volume (a hybrid filesystem).

The sample creates three users with different roles (guest, user and administrator).

======  ========  =======
User    Password  Role
------  --------  -------
guest   (none)    Guest
me      "123"     User
admin   "1234"    Admin
======  ========  =======

You'll need to have WiFi configured. You can set this information when building like this::

   make WIFI_SSID=ssid WIFI_PWD=password

substituting your actual Access Point details for *ssid* and *password*.

Flash to your device::

   make flash

You should be able to connect using an FTP client:

   ftp ipaddress

and when prompted log in with one of the above usernames.
