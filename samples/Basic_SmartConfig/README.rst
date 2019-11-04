Basic Smart Config
==================

Introduction
------------

SmartConfig is a mechanism to more easily configure an ESP device using a smart phone.

Calling `smartConfigStart()` starts a search for an Access Point (AP) with
a special signature. It then tries to extract data like SSID and password from it.
The App on your smart phone sends out that information.

The example here shows how to use ESP_TOUCH method to do smart
configuration on the device. It ported from the C code that
Espressif provides in the SDK examples.

You will need an App for your Smart Phone, such as:

-  Android https://github.com/EspressifApp/EsptouchForAndroid
-  iOS https://github.com/EspressifApp/EsptouchForIOS
-  ESP8266 SmartConfig (search on Android Play)

See also https://www.espressif.com/en/products/software/esp-touch/overview.
