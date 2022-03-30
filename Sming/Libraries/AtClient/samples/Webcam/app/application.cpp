#include <SmingCore.h>
#include <AtClient.h>

AtClient* atClient;

namespace
{
int pictureSize;

bool processSize(AtClient& atClient, String& reply)
{
	/*
	 * ================
		AT+CAMCAP capture picture
		Set Command:
		Capture and store a Picture local and returns bytes of captured picture
		AT+CAMCAP
		+CAMCAP:<bytes>
		OK
		on error:
		+CME ERROR:
	* ================
	*/
	if(reply.indexOf("+CME ERROR") == 0) {
		debug_e("Unable to capture image");
		return false;
	}

	// Read the size of the captured image
	String length = reply.substring(8);
	pictureSize = atoi(length.c_str());

	debug_d("Picture size: %d", pictureSize);

	return true;
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);
	Serial.swap(); //swap to GPIO13 and GPIO15

	atClient = new AtClient(Serial);

	// The commands below will be queued and executed
	// in the same order as defined below.
	// One command needs to finish successfully in order for the next one to start.
	atClient->send("ATE0\r");
	atClient->send("AT+CAMSTOP\r");

	atClient->send("AT+CAMSTART=1\r");
	atClient->send("AT+CAMCAP\r", processSize);
}
