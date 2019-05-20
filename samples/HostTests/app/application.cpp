#include <SmingCore.h>

extern void test_json();
extern void test_files();

void init()
{
	spiffs_mount();
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	test_json();
	test_files();

	system_restart();
}
