Google-Cast
===========

.. highlight:: bash

This component allows you to communicate with Chrome Cast dongles or smart TVs that support the Google Cast Protocol.

Using
-----

1. Add ``COMPONENT_DEPENDS += google-cast`` to your application componenent.mk file.
2. Add these lines to your application::

      #include <Network/GoogleCast/Client.h>

3. Example::

      #include <Network/GoogleCast/Client.h>

      GoogleCast::Client castClient;

      void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
      {
         // connect directly to the IP of the devise
         castClient.connect(IpAddress("192.168.10.15"));

         // Starting YouTube on the device
         castClient.launch("YouTube");
         castClient.setResponseHandler(onMessage);
      }

Re-Generating C files from proto file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
You can re-generate the C files from the proto file. This can be useful if you want to use a newer version of the Google Cast proto file.
Make sure to compile it using `nanopb`. The re-generation can be done with the commands below::

    cd $SMING_HOME/Libraries/google-cast/sample
    make python-requirements # should be executed once to download and install the required python packages
    make rebuild-cast-proto

Protocol
--------
There are multiple documents in internet that describe the Google Cast protocol and its inner workings. As a start one can take a look at
this presentation and read this documentation.

# Presentation with insights
https://docs.google.com/presentation/d/1X1BdFunVnLkF7L0BgevH2zzkcSe0_gtdTJ_pMdEuakQ/htmlpresent


# Protocol description
https://github.com/thibauts/node-castv2#the-tls--protocol-buffers-layer

# Application in C++/Qt
https://github.com/jhenstridge/cast-app

# App-Ids
List of current APP-IDS
https://clients3.google.com/cast/chromecast/device/baseconfig

These two URLs will output JSON in the format of the App List

Individual app configurations are obtained from this url:

https://clients3.google.com/cast/chromecast/device/app?a={appid}

Where {appid} is the id of the app, and the output of this is JSON in the format of a receiver app defintion.
