#include <SmingCore.h>

#include <Libraries/CapacitiveSensor/CapacitiveSensor.h>

// http://playground.arduino.cc/Main/CapacitiveSensor?from=Main.CapSense
// I'm using a 250K resistor currently. Using 500K seems to timeout. I'm guessing that the higher
// clock speed on the ESP means we need a higher charge current than arduino ??
// Further investigation required.

#define PIN_SEND 0
#define PIN_RECEIVE 2
#define SAMPLES_TO_READ 30

namespace
{
CapacitiveSensor cs_0_2(PIN_SEND, PIN_RECEIVE);
SimpleTimer procTimer;

void capsense()
{
	long total = cs_0_2.capacitiveSensor(SAMPLES_TO_READ);
	Serial << _F("Sense Value: ") << total << endl;
}

} // namespace

void init()
{
	procTimer.initializeMs<100>(capsense).start();
}
