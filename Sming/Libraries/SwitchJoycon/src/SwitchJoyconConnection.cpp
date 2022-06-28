#include "SwitchJoyconConnection.h"

void SwitchJoyconConnection::onConnect(NimBLEServer* server)
{
	connected = true;
	if(connectCallback) {
		connectCallback(*server);
	}
}

void SwitchJoyconConnection::onDisconnect(NimBLEServer* server)
{
	connected = false;
	if(connectCallback) {
		disconnectCallback(*server);
	}
}
