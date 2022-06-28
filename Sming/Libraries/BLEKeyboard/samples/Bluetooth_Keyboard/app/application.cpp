#include <SmingCore.h>
#include <BleKeyboard.h>

namespace
{
BleKeyboard bleKeyboard("Sming BLE Keyboard");
Timer procTimer;

void loop()
{
	if(!bleKeyboard.isConnected()) {
		return;
	}

	Serial.println("Sending 'Hello world'...");
	bleKeyboard.print("Hello world");

	Serial.println("Sending Enter key...");
	bleKeyboard.write(KEY_RETURN);

	delay(1000);

	Serial.println("Sending Play/Pause media key...");
	bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);

	//
	// Below is an example of pressing multiple keyboard modifiers
	// which by default is commented out.
	/*
	 Serial.println("Sending Ctrl+Alt+Delete...");
	 bleKeyboard.press(KEY_LEFT_CTRL);
	 bleKeyboard.press(KEY_LEFT_ALT);
	 bleKeyboard.press(KEY_DELETE);
	 delay(100);
	 bleKeyboard.releaseAll();
	 */
}

} // namespace

void init()
{
	Serial.begin(COM_SPEED_SERIAL);
	Serial.println("Starting BLE Keyboard sample!");
	bleKeyboard.begin();

	procTimer.initializeMs(1000, loop).start();
}
