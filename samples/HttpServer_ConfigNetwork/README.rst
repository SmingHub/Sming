HttpServer Config Network
=========================

Introduction
------------

The HTTP server coming with Sming is quite powerful but it is limited
from the available resources of the underlining hardware (your favorite
ESP8266 microcontroller). Serving multiple files at once can be
problematic. It is not the size of the files that can cause problems,
but the number of simultaneous files that need to be delivered.
Therefore if you serve multiple CSS or JS files you can optimize your
web application before uploading it into your ESP8266 using the advice
below.

Optimizing File Delivery
------------------------

In this example you will see how to combine CSS and JS files, compress
them and deliver the optimized content via the HTTP server.

Installation
~~~~~~~~~~~~

The file optimization uses ``gulp``. To install it and the needed gulp
packages you need to install first `npm <https://www.npmjs.com/>`__. Npm
is the Node.JS package manager. Once you are done with the installation
you can run from the command line the following:

npm install

The command above will install gulp and its dependencies.

Usage
~~~~~

During the development of your web application you should work only in
the ``web/dev/`` folder. Once you are ready with the application you can
``pack`` the resources and ``upload`` them to your device. The commands
are

make web-pack make web-upload

That should be it.
