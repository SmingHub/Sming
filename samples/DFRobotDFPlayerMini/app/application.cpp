using namespace std;
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Sming/Libraries/DFRobotDFPlayerMini/DFRobotDFPlayerMini.h>

#include <string>
#include <string.h>

void init()
{
    Serial.begin(9600);
    myDFPlayer.begin(Serial);
    myDFPlayer.volume(10);
    myDFPlayer.play(1);
}
