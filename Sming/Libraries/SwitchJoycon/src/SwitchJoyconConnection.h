#pragma once

#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "nimconfig.h"
#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)

#include <NimBLEServer.h>
#include <NimBLECharacteristic.h>

class SwitchJoyconConnection : public NimBLEServerCallbacks
{
public:
	using Callback = Delegate<void(NimBLEServer& server)>;

	bool connected{false};
	NimBLECharacteristic* inputGamepad{nullptr};

	SwitchJoyconConnection(Callback onConnected = nullptr, Callback onDisconnected = nullptr)
	{
		connectCallback = onConnected;
		disconnectCallback = onDisconnected;
	}

	void onConnect(NimBLEServer* pServer);
	void onDisconnect(NimBLEServer* pServer);

private:
	Callback connectCallback;
	Callback disconnectCallback;
};

#endif // CONFIG_BT_NIMBLE_ROLE_PERIPHERAL
#endif // CONFIG_BT_ENABLED
