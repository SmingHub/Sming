# AM2321

- AM2321 sensor support for Arduino. 
- More: http://playground.arduino.cc/Main/AM2321

## Example:

	#include <AM2321.h>
	
	void readByAM2321()
	{
		AM2321 am2321;
		am2321.read();
	
	    Serial.print("(");
	    Serial.print(am2321.temperature/10.0);
	    Serial.print(", ");
	    Serial.print(am2321.humidity/10.0);
	    Serial.println(')');
	}
