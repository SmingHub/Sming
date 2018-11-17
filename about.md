---
layout: page
title: About
permalink: /about/
---

# Sming Framework
Sming - Open Source framework for high efficiency WiFi SoC ESP8266 native development with C++ language.

## Summary
* Highly effective in performance and memory usage (this is native firmware!)
* Simple and powerful hardware API wrappers
* Fast & user friendly development
* Work with GPIO in Arduino style
* Compatible with standard Arduino libraries - use any popular hardware in few lines of code
* Integrated boot loader [rBoot](https://github.com/raburton/rboot) with support for 1MB ROMs, OTA firmware updating and ROM switching
* Built-in file system: [spiffs](https://github.com/pellepl/spiffs)
* Built-in powerful wireless modules
* Powerful asynchronous (async) network stack
    * Async TCP and UDP stack based on [LWIP](http://savannah.nongnu.org/projects/lwip/)
    * With clients supporting: HTTP, MQTT, WebSockets and SMTP
    * And servers for: DNS, FTP, HTTP(+ WebSockets), Telnet
    * With SSL support for all network clients and servers based on [axTLS 2.1+](https://github.com/igrr/axtls-8266) with [Lwirax](https://github.com/attachix/lwirax/).
    * Out of the box support for OTA over HTTPS.
* Crash handlers for analyzing/handling system restarts due to fatal errors or WDT resets.
* PWM support based on [Stefan Bruens PWM](https://github.com/StefanBruens/ESP8266_new_pwm.git)
* Optional custom heap allocation based on [Umm Malloc](https://github.com/rhempel/umm_malloc.git)
* Based on Espressif NONOS SDK. Tested with versions 1.5 and 2.0. Experimental support for SDK version >= 3.0.0.

## Compatibility

OS/SDK | Linux | Mac OS X | Windows | FreeBSD-current |
-------|-------|----------|---------|-----------------|
UDK (v1.5)    | n/a   | n/a      |   [![Build status](https://ci.appveyor.com/api/projects/status/5aj0oi0wyk4uij00/branch/develop?svg=true)](https://ci.appveyor.com/project/slaff/sming-sb483/branch/develop)      |     n/a         |
esp-open-sdk (v1.5, v2.0, v3.0 **) | :sunny:  | :sunny: | n/a | :sunny: |

- OS = Operating System.
- SDK = Software Development Kit.
- n/a = The selected SDK is not available on that OS.
- ** = experimental support.

Further details can be found in the official development site of the project [https://github.com/SmingHub/Sming](https://github.com/SmingHub/Sming).
