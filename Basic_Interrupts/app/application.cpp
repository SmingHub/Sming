#include <user_config.h>
#include <SmingCore/SmingCore.h>

#define INT_PIN 0   // GPIO0
#define say(a) ( Serial.print(a) )

void IRAM_ATTR interruptHandler()
{
	// This is not very good idea - make actions in interrupt handler (it's just an example)
	// Better set some flag and check it in main code (timers and etc) or read actual pins state and save it to variable
	// Interrupt processing code should be as short as possible.
	// flagInterruptOccurred = true;
	
	say(micros());
	say( "   Pin changed, now   ");
	say( digitalRead(INT_PIN));
	Serial.println();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 or 9600 by default

	delay(3000);
	say("=========================== Bring GPIO");
	say( INT_PIN);
	say( " low to trigger interrupt(s) ========================");
	Serial.println();

	attachInterrupt(INT_PIN, interruptHandler, CHANGE);
}
