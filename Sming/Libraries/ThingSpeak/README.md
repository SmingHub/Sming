# Sming - ThingSpeak driver libary
## Usage
### Declatration - init

First way set write APIKey in the constructor.
```C++
ThingSpeak thingSpeak("APIKey"); //String of ThingSpeak APIKey
```
second way is set in the function:

```C++
ThingSpeak thingSpeak; //free constructor
thingSpeak.setWriteAPIKey( APIKey ); // APIKey - String varible of Write API TS Key
```
You can use `setWriteAPIKey` whenever you need, while changing API by user etc.

### Usage in loop
#### Add a field value
If you are adding value to the specific fields you can use `setField(iField, value)`
* `iField` is field id (1-8)
* `value` is value of field data (int, float, String)

Or if you want to add a field sequence use `setNextField(value)`
* `value` is value of field data (int, float, String)

this function add a value of next field (1, 2, 3, 4, ...) if you use first `setField` and add a field to a specific number, eg 4
function `setNextField` add value to next field (5)

#### Send values for server
for send all added values and clear fields to next mesures use `sendFields()`

#### Clear fields
to clear all field value use `cleanFields()`

### Example

```C++
#include <SmingCore/SmingCore.h>
#include <Libraries/ThingSpeak/ThingSpeak.h>

Timer procTimer;
ThingSpeak thingSpeak("YouTsAPIKey");
uint16_t value;

void readData() //Timered sensor read theard
{
	value = system_adc_read();      // Read value from ESP ADC
	thingSpeak.setField(1, value);  // Add value to TS libary
	thingSpeak.sendFields();        // Send all added values
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);           // 115200 by default
	Serial.systemDebugOutput(true);           // Allow debug output to serial
	WifiStation.enable(true);                 // Enable WiFi
	WifiStation.config(WIFI_SSID, WIFI_PWD);  // You pass to WiFi

	procTimer.initializeMs(20 * 1000, readData).start(); 
}
```
