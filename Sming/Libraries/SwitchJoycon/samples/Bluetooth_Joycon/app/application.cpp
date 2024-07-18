#include <SmingCore.h>
#include <SwitchJoycon.h>

namespace
{
Timer procTimer;

void onConnect(NimBLEServer& server);
void onDisconnect(NimBLEServer& server);

SwitchJoycon joycon(SwitchJoycon::Type::Left, 100, onConnect, onDisconnect);

void loop()
{
	if(!joycon.isConnected()) {
		return;
	}

	uint8_t button = random(0, 15);

	joycon.press(button);
	joycon.setHat(SwitchJoycon::JoystickPosition::UpLeft);
	delay(5000);

	joycon.release(button);
	joycon.setHat(SwitchJoycon::JoystickPosition::Center);
	delay(5000);
}

void onConnect(NimBLEServer& server)
{
	Serial.println("Connected :) !");

	procTimer.initializeMs<500>(loop).start();
}

void onDisconnect(NimBLEServer& server)
{
	procTimer.stop();
	Serial.println("Disconnected :(!");
}

} // namespace

void init()
{
	Serial.begin(COM_SPEED_SERIAL);
	Serial.systemDebugOutput(true);

	Serial.println("Starting Joycon Gamepad sample!");
	joycon.begin();
	// Auto reporting is enabled by default.
	// Use joycon.setAutoReport(false); to disable auto reporting, and then use joycon.sendReport(); as needed
}
