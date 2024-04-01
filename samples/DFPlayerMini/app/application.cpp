#include <SmingCore.h>
#include <Libraries/DFRobotDFPlayerMini/DFRobotDFPlayerMini.h>

#define GPIO_LED 2

namespace
{
SimpleTimer timer;
DFRobotDFPlayerMini player;

void nextSong()
{
	player.next();
}

void checkReady()
{
	static bool ledState;

	ledState = !ledState;
	digitalWrite(GPIO_LED, ledState);

	if(!player.begin(Serial)) {
		return;
	}

	digitalWrite(GPIO_LED, 0);

	player.volume(15);

	timer.initializeMs<10000>(nextSong).start();
}

} // namespace

void init()
{
	Serial.begin(9600);

	pinMode(GPIO_LED, OUTPUT);
	timer.initializeMs<250>(checkReady).start();
}
