Smart Config
============

Introduction
------------

SmartConfig is a mechanism to configure a device as quickly as possible
with the intermediate help of a smart phone and with least interaction
from a human as possible.

Basically your ESP device looks for Access Points (AP). When it finds an
AP with special signature it tries to extract data like SSID and
password from it. Meanwhile your smart phone tries to send that
information.

The example here shows how to use ESP_TOUCH method to do smart
configuration on the device. It is a C++ conversion of the C code that
Espressif provides in the SDK examples.

| What you will need also is the code that has to be run on your smart
  phone. Espressif already released some sample code and you can try:
| \* Android - https://github.com/EspressifApp/EsptouchForAndroid \* iOS
  - https://github.com/EspressifApp/EsptouchForIOS
