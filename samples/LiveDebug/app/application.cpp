#include <user_config.h>
#include <SmingCore.h>
#include "HardwareTimer.h"

#define LED_PIN 2 // Note: LED is attached to UART1 TX output

#define TIMERTYPE_HARDWARE 1
#define TIMERTYPE_SIMPLE 2
#define TIMERTYPE_TIMER 3

/*
 * This example uses the hardware timer for best timing accuracy. There is only one of these on the ESP8266,
 * so it may not be available if another module requires it.
 * Most timing applications can use a SimpleTimer, which is good for intervals of up to about 268 seconds.
 * For longer intervals, use a Timer.
 */
#define TIMER_TYPE TIMERTYPE_HARDWARE

/*
 * HardwareTimer defaults to non-maskable mode, so the timer callback cannot be interrupted even by the
 * debugger. To use break/watchpoints we must set the timer to use maskable mode.
 */
#define HWTIMER_TYPE eHWT_Maskable

#if TIMER_TYPE == TIMERTYPE_HARDWARE
HardwareTimer procTimer(HWTIMER_TYPE);
// Hardware timer callbacks must always be in IRAM
#define CALLBACK_ATTR IRAM_ATTR
#elif TIMER_TYPE == TIMERTYPE_SIMPLE
SimpleTimer procTimer;
#define CALLBACK_ATTR GDB_IRAM_ATTR
#else
Timer procTimer;
#define CALLBACK_ATTR GDB_IRAM_ATTR
#endif

bool state = true;

/*
* Notice:  Software breakpoints work only on code that is in RAM.
*          In Sming you have to use the GDB_IRAM_ATTR to do this.
*/
void CALLBACK_ATTR blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
}

void onDataReceived(Stream& source, char arrivedChar, unsigned short availableCharsCount)
{
	if(arrivedChar == '\n' && availableCharsCount != 0) {
		char buffer[availableCharsCount];
		auto count = Serial.readMemoryBlock(buffer, availableCharsCount);
		Serial.print(_F("You typed: "));
		Serial.write(buffer, count);
	}
}

void GDB_IRAM_ATTR init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.onDataReceived(onDataReceived);
	pinMode(LED_PIN, OUTPUT);
#if TIMER_TYPE == TIMERTYPE_SIMPLE
	procTimer.setCallback(SimpleTimerCallback(blink));
	procTimer.startMs(1000, true);
#else
	procTimer.initializeMs(1000, blink).start();
#endif
}
