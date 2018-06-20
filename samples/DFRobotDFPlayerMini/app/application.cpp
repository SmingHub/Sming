using namespace std;
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/DFRobotDFPlayerMini/DFRobotDFPlayerMini.h>

#include <string>
#include <string.h>

DFRobotDFPlayerMini player;

void init()
{
    Serial.begin(9600);
    player.begin(Serial);
    player.volume(10);
    player.play(1);
}
