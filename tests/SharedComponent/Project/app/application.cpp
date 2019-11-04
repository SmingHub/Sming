#include <SmingCore.h>

#include "print-test.h"

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	print_test();

	system_restart();
}
