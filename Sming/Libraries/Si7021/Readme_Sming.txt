This library was adopted from:

https://github.com/jayjayuk/Si7021-Humidity-And-Temperature-Sensor-Library

It mostly works (run examples SI7021_*).

Issues on my device (ebay, sold as Si7021 but getDeviceID() reports Si7020):

- readTemp() following readHumidity() doesn't work, but sequence readHumidity(), readTempPrev() does,
- setHeater() works, but getHeater() always reports heater on,


