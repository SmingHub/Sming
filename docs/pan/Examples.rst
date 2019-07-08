Basic_Serial
============

Very basic example of UART communication. Sming allows you to work in an
Arduino-like style.

Basic_Blink
===========

Simple blink example. We use Timer instead of a loop because we want to
allow WiFi communications to work in the background.

|ESP8266 Blink example|

Basic_WiFi
==========

WiFi network connection and scanning functionality. Software Access
Point.

Basic_ScannerI2C
================

Classic Arduino I2C scanner with adaptations for Sming framework.

Humidity_DHT22
==============

ESP8266 DHT22 sensor reader.

|ESP8266 DHT22 sensor example|

Temperature_DS1820
==================

ESP8266 DS1820/DS18B20 sensor reader.

|ESP8266 DS1820 example|

LiquidCrystal_44780
===================

ESP8266 LiquidCrystal screen I2C connection example.

|ESP8266 LiquidCrystal 44780 example|

Pressure_BMP180
===============

ESP8266 BMP180 sensor reader.

|ESP8266 BMP180 example|

Compass_HMC5883L
================

ESP8266 HMC5883L sensor reader.

|ESP8266 HMC5883L example|

HttpClient_ThingSpeak
=====================

Example of HttpClient and direct ESP8266 ThingSpeak data pushing.

|ESP8266 ThingSpeak example|

HttpServer_Bootstrap
====================

ESP8266 embedded web server.

At start ESP will download all requried files from remote server. All
content, including JQuery and Bootstrap will be saved on device (and can
work offline). Static files stored in GZIPed mode.

|ESP8266 WEB Server code|

Light_BH1750
============

ESP8266 BH1750 sensor reader.

|ESP8266 BH1750 example|

ScreenOLED_SSD1306
==================

Example of direct work with SSD1306 OLED screen on ESP8266.

Minimal requirements: **ESP-03**

|ESP8266 SSD1306 OLED screen example|

ScreenTFT_ILI9163C
==================

Example of direct work with ILI9163C 1.44" TFT screen on ESP8266.

Minimal requirements: **ESP-03**

|ESP8266 ILI9163C TFT screen example|

MeteoControl
============

More complex example of Internet of Things device. Can read humidity and
temperature data from sensor and output it to screen with actual time.
Time loaded directly from Google. Also with device can automatically
control any external load.

**Features:** \* temperature \* humidity \* actual time from internet \*
build-in display \* web control interface \* automatically control
external load \* HTTP API for data reading & writing \* initialization
from internet at first run

|ESP8266 MeteoControl IoT Device| |ESP8266 MeteoControl IoT Device|

|ESP8266 MeteoControl IoT Device| |ESP8266 MeteoControl IoT Device|

**How web interface looks like:**

|ESP8266 web server interface|

.. |ESP8266 Blink example| image:: images/small/Blink.JPG
   :target: images/Blink.JPG
.. |ESP8266 DHT22 sensor example| image:: images/small/DHT22.JPG
   :target: images/DHT22.JPG
.. |ESP8266 DS1820 example| image:: images/small/DS1820.JPG
   :target: images/DS1820.JPG
.. |ESP8266 LiquidCrystal 44780 example| image:: images/small/LiquidCrystal.JPG
   :target: images/LiquidCrystal.JPG
.. |ESP8266 BMP180 example| image:: images/small/BMP180.JPG
   :target: images/BMP180.JPG
.. |ESP8266 HMC5883L example| image:: images/small/HMC5883L.JPG
   :target: images/HMC5883L.JPG
.. |ESP8266 ThingSpeak example| image:: images/small/ThingSpeak.png
   :target: images/ThingSpeak.png
.. |ESP8266 WEB Server code| image:: images/small/Esp8266WebServer.png
   :target: images/Esp8266WebServer.png
.. |ESP8266 BH1750 example| image:: images/small/BH1750.JPG
   :target: images/BH1750.JPG
.. |ESP8266 SSD1306 OLED screen example| image:: images/small/SSD1306.JPG
   :target: images/SSD1306.JPG
.. |ESP8266 ILI9163C TFT screen example| image:: images/small/ILI9163C.JPG
   :target: images/ILI9163C.JPG
.. |ESP8266 MeteoControl IoT Device| image:: images/small/MeteoControl_IoT_device_1.JPG
   :target: images/MeteoControl_IoT_device_1.JPG
.. |ESP8266 MeteoControl IoT Device| image:: images/small/MeteoControl_IoT_device_3.JPG
   :target: images/MeteoControl_IoT_device_3.JPG
.. |ESP8266 MeteoControl IoT Device| image:: images/small/MeteoControl_IoT_device_2.JPG
   :target: images/MeteoControl_IoT_device_2.JPG
.. |ESP8266 MeteoControl IoT Device| image:: images/small/MeteoControl_IoT_device_4.JPG
   :target: images/MeteoControl_IoT_device_4.JPG
.. |ESP8266 web server interface| image:: images/small/MeteoControl_IoT_device_config.png
   :target: images/MeteoControl_IoT_device_config.png
