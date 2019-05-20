#include <SmingCore.h>

#include <Libraries/CapacitiveSensor/CapacitiveSensor.h>

// http://playground.arduino.cc/Main/CapacitiveSensor?from=Main.CapSense
// Im using a 250K resistor currently. Using 500K seems to timeout. Im guessing that the higher
// Clock speed on the ESP means we need a higher charge current than arduino ??
// Further investigation required.

CapacitiveSensor cs_0_2 = CapacitiveSensor(0, 2); //Send pin 0, Receive Pin 2.

Timer procTimer;

void capsense()
{
	long total = cs_0_2.capacitiveSensor(30); //Read sensor with 30 samples
	Serial.print("Sense Value: ");
	Serial.println(total); // print sensor output
}

void init()
{
	procTimer.initializeMs(100, capsense).start();
}
