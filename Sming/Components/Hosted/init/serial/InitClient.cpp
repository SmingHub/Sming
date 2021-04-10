#include <SmingCore.h>
#include <Hosted/Client.h>

Hosted::Client* hostedClient{nullptr};

extern void init();

void host_init()
{
	Serial.begin(115200);
	hostedClient = new Hosted::Client(Serial);
	init();
}
