#include <SmingCore.h>

extern void test_json5();
extern void test_json6();
extern void test_files();

void init()
{
	spiffs_mount();
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	test_json5();
	test_json6();
	test_files();

	system_restart();
}
