#include <SmingCore.h>
#include <BleGamepad.h>

namespace
{
BleGamepad bleGamepad;

Timer procTimer;

void loop()
{
	if(!bleGamepad.isConnected()) {
		return;
	}

	Serial.println("Press buttons 5 and 16. Move all enabled axes to max. Set DPAD (hat 1) to down right.");
	bleGamepad.press(BUTTON_5);
	bleGamepad.press(BUTTON_16);
	bleGamepad.setAxes(32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767);
	bleGamepad.setHat1(DPAD_DOWN_RIGHT);
	// All axes, sliders, hats etc can also be set independently. See the IndividualAxes.ino example
	delay(500);

	Serial.println("Release button 5. Move all axes to min. Set DPAD (hat 1) to centred.");
	bleGamepad.release(BUTTON_5);
	bleGamepad.setAxes(-32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767);
	bleGamepad.setHat1(DPAD_CENTERED);
	delay(500);
}

} // namespace

void init()
{
	Serial.begin(COM_SPEED_SERIAL);
	Serial.println("Starting BLE Gamepad sample!");
	bleGamepad.begin();
	// The default bleGamepad.begin() above is the same as bleGamepad.begin(16, 1, true, true, true, true, true, true, true, true, false, false, false, false, false);
	// which enables a gamepad with 16 buttons, 1 hat switch, enabled x, y, z, rZ, rX, rY, slider 1, slider 2 and disabled rudder, throttle, accelerator, brake, steering
	// Auto reporting is enabled by default.
	// Use bleGamepad.setAutoReport(false); to disable auto reporting, and then use bleGamepad.sendReport(); as needed

	procTimer.initializeMs<500>(loop).start();
}
