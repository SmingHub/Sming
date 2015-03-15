#include "../SmingCore/SmingCore.h"

Timer procTimer;

bool ok = false;
void onTimer()
{
	ok = !ok;
	digitalWrite(0, ok);
	digitalWrite(2, !ok);
}

void init()
{
	Serial.begin(230400);
	Serial.systemDebugOutput(true);
	Serial.println("Hello friendly world! :)");


	debugf("ST");
	char json[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";

	DynamicJsonBuffer jsonBuffer;

	JsonObject& root = jsonBuffer.parseObject(json);

	const char* sensor = root["sensor"];
	long timese          = root["time"];
	double latitude    = root["data"][0];
	double longitude   = root["data"][1];

	debugf("%s, %d %s", sensor, timese, String(longitude, 12).c_str());
	debugf("OK");

	{
		JsonObject& root = jsonBuffer.createObject();
		root["sensor"] = "gps";
		root["time"] = 1351824121;

		JsonArray& data = root.createNestedArray("data");
		data.add(48.756080, 6);  // 6 is the number of decimals to print
		data.add(2.302038, 6);   // if not specified, 2 digits are printed

		root.printTo(Serial);
	}

	pinMode(0, OUTPUT);
	pinMode(2, OUTPUT);
	procTimer.initializeMs(10, onTimer).start();
}
