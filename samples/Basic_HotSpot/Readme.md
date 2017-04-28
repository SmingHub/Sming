## Introduction
A sample that will turn your ESP8266 device into WIFI forwarder.

This means that it will connect to your WIFI router, turn on a HotSpot 
and you can connect to the HotSpot.

WIFI router <---> (station mode) ESP8266 (HotSpot) <---> PC/Mobile Phone/Table/etc.

It can be used as simple WIFI range extender or repeater.

## Prerequisites

This feature requires LWIP to be compiled with specific settings. Namely ENABLE_CUSTOM_LWIP=1 and ENABLE_IP_FORWARD=1.
The easiest way to recompile LWIP with the right settings is to run the following commands:
```
cd $SMING_HOME/../samples/Basic_HotStop
make -C $SMING_HOME dist-clean
make ENABLE_CUSTOM_LWIP=1 ENABLE_IP_FORWARD=1
```

Make sure also to set the correct SSID and password for the WIFI router. 
Your new hotspot will be named `SmingAP`.
