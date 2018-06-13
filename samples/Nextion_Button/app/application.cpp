using namespace std;
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/ITEADLIB_Arduino_Nextion/Nextion.h>

#include <string>
#include <string.h>

#define GPIO_LED 2

// See this example in action: https://youtu.be/lHk6fqDBHyI
// The HMI file included in this example needs to be compiled
// with the Nextion Editor and uploaded to the Nextion display
// using standard method.
// On WEMOS mini D1 (where this example was tested), the
// Nextion device is connected to RX/TX pins as required.
// BUT it needs to be disconnected when uploading the firmware.
// So the process is:
// 1. Make changes to the cpp code
// 2. Build it using "make"
// 3. Disconnect the Nextion display if it is connected to Rx/Tx.
// 4. Upload firmware (built in step 2) using "make flash".
// 5. Connect Nextion display back again to Rx/Tx.
// Note: I always unplugged the ESP8266 from USB (connecting with computer)
//       while fiddling with the connections between ESP8266 and Nextion display.

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
