#include <user_config.h>
#include <SmingCore.h>
#include <Libraries/DFRobotDFPlayerMini/DFRobotDFPlayerMini.h>

#define GPIO_LED 2

Timer timerDFPlayer;
Timer timerLedBlink;
DFRobotDFPlayerMini player;
bool ledState = true;

void blink()
{
	digitalWrite(GPIO_LED, ledState);
	ledState = !ledState;
}

void nextSong()
{
	player.next();
}

void init()
{
	Serial.begin(9600);

	pinMode(GPIO_LED, OUTPUT);
	timerLedBlink.initializeMs(100, blink).start();

	while(!player.begin(Serial)) {
		delay(500);
	}

	timerLedBlink.stop();
	digitalWrite(GPIO_LED, 0);

	player.volume(15);

	timerDFPlayer.initializeMs(10000, nextSong).start();
}
