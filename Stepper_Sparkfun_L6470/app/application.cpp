#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Sparkfun_L6470/Sparkfun_L6470.h>
//#include <Libraries/Sparkfun_L6470/SparkFundSPINConstants.h>


// Put you SSID and Password here
#define WIFI_SSID "AndroidAP"
#define WIFI_PWD "Doitman1"
#define NOTE_DIVISOR 2  // My cheesy way of reducing the note frequencies to a ranges


/* Pinout:
int8_t  MISO = 12;
int8_t  MOSI= 13;
int8_t  CLK= 14;
int8_t  CS =15;
int8_t  DC =5;
int8_t  RST= 4;
*/

//Adafruit_ILI9341 tft;
//Sparkfun_L6470(int8_t SCLK, int8_t DIN, int8_t DC, int8_t RST);

Sparkfun_L6470 stepper = Sparkfun_L6470(14, 12, 5, 4);

Timer guiTimer;
int stepDir = 1;

void dSPINConfig(void)
{
    stepper.configSyncPin(BUSY_PIN, 0); // BUSY pin low during operations;
                                        //  second paramter ignored.
    stepper.configStepMode(STEP_FS);   // 0 microsteps per step
    stepper.setMaxSpeed(10000);        // 10000 steps/s max
    stepper.setFullSpeed(10000);       // microstep below 10000 steps/s
    stepper.setAcc(10000);             // accelerate at 10000 steps/s/s
    stepper.setDec(10000);
    stepper.setSlewRate(SR_530V_us);  // Upping the edge speed increases torque.
    stepper.setOCThreshold(OC_750mA);  // OC threshold 750mA
    stepper.setPWMFreq(PWM_DIV_2, PWM_MUL_2); // 31.25kHz PWM freq
    stepper.setOCShutdown(OC_SD_DISABLE); // don't shutdown on OC
    stepper.setVoltageComp(VS_COMP_DISABLE); // don't compensate for motor V
    stepper.setSwitchMode(SW_USER);    // Switch is not hard stop
    stepper.setOscMode(INT_16MHZ_OSCOUT_16MHZ); // for boardA, we want 16MHz
//  internal osc, 16MHz out. boardB and
//  boardC will be the same in all respects
//  but this, as they will bring in and
//  output the clock to keep them
//  all in phase.
    stepper.setAccKVAL(255);        // We'll tinker with these later, if needed.
    stepper.setDecKVAL(255);
    stepper.setRunKVAL(255);
    stepper.setHoldKVAL(32);  // This controls the holding current; keep it low.
}

void playNote()
{
    if (stepDir == 1)
    {
        Serial.println("Direction=1");
        stepper.run(FWD, 40);
        stepDir = 0;
    }
    else
    {
        Serial.println("Direction=0");
        stepper.run(REV, 200);
        stepDir = 1;
    }

}


void basicGui()
{
    //playNote();
    Serial.println("runned");
    stepper.run(FWD, 40);
}


void init()
{
	WDT.enable(false);
    Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
    Serial.systemDebugOutput(true); // Allow debug output to serial
    WifiStation.config(WIFI_SSID, WIFI_PWD);
    WifiStation.enable(true);
    //WifiAccessPoint.enable(false);
    //delay(3000);
    Serial.println("L6470 test start");
    Serial.println("Config start");
    dSPINConfig();
    Serial.println("Config end.");

    //guiTimer.initializeMs(2000, basicGui).start();
    WifiStation.waitConnection(basicGui, 30, playNote);
}
