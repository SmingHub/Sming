using namespace std;
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/DFRobotDFPlayerMini/DFRobotDFPlayerMini.h>

#include <string>
#include <string.h>

#define GPIO_LED 2

Timer timerDFPlayer;
DFRobotDFPlayerMini player;
void nextSong()
{
    player.next();
}


Timer timerLedBlink;
bool ledState = true;
void blink()
{
    digitalWrite(GPIO_LED, ledState);
    ledState = !ledState;
}


void init()
{
    Serial.begin(9600);
    
    pinMode(GPIO_LED, OUTPUT);
    timerLedBlink.initializeMs(100, blink).start();

    while(!player.begin(Serial))
    {
        delay(500);
    }

    timerLedBlink.stop();
    digitalWrite(GPIO_LED, 0);

    player.volume(15);

    timerDFPlayer.initializeMs(10000, nextSong).start();

}
