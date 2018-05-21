using namespace std;
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/ITEADLIB_Arduino_Nextion/Nextion.h>

#include <string>
#include <string.h>

#define GPIO_LED 2


NexButton b0 = NexButton(0, 1, "b0");
NexText t0 = NexText(0, 2, "g0");

NexTouch *nex_listen_list[] = 
{
        &b0,
        NULL
};

Timer timerNextion;

void loopNextion()
{
    nexLoop(nex_listen_list);
}

bool ledState = true;

void b0PopCallback(void *ptr)
{
    digitalWrite(GPIO_LED, ledState);
    // state == false => on
    // state == true  => off
    if(!ledState)
    {
        t0.setText("ESP8266 says: LED is on");
    }
    else
    {
        t0.setText("ESP8266 says: LED is off");
    }
    ledState = !ledState;
}

void init()
{
    pinMode(GPIO_LED, OUTPUT);
    nexInit();
    b0.attachPop(b0PopCallback, &b0);
    timerNextion.initializeMs(100, loopNextion).start();
}
