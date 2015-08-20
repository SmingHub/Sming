#ifndef Sparkfun_L6470_h
#define Sparkfun_L6470_h

#include "Arduino.h"
#include "SparkFundSPINConstants.h"
//#include "ESP8266_SPI.h"
#include "SPI.h"

class Sparkfun_L6470
{
  public:
    // Constructors. We'll ALWAYS want a CS pin and a reset pin, but we may
    //  not want a busy pin. By using two constructors, we make it easy to
    //  allow that.
    Sparkfun_L6470(uint8_t CSPin, uint8_t resetPin, uint8_t busyPin);
    Sparkfun_L6470(uint8_t CSPin, uint8_t resetPin);
    
    // These are super-common things to do: checking if the device is busy,
    //  and checking the status of the device. We make a couple of functions
    //  for that.
    uint8_t busyCheck();
    uint8_t getStatus();
    
    // Some users will want to do things other than what we explicitly provide
    //  nice functions for; give them unrestricted access to the parameter
    //  registers.
    void setParam(byte param, unsigned long value);
    unsigned long getParam(byte param);
    
    // Lots of people just want Commands That Work; let's provide them!
    // Start with some configuration commands
    void setLoSpdOpt(boolean enable);
    void configSyncPin(byte pinFunc, byte syncSteps);
    void configStepMode(byte stepMode);
    void setMaxSpeed(float stepsPerSecond);
    void setMinSpeed(float stepsPerSecond);
    void setFullSpeed(float stepsPerSecond);
    void setAcc(float stepsPerSecondPerSecond);
    void setDec(float stepsPerSecondPerSecond);
    void setOCThreshold(byte threshold);
    void setPWMFreq(uint16_t divisor, uint16_t multiplier);
    void setSlewRate(uint16_t slewRate);
    void setOCShutdown(uint8_t OCShutdown);
    void setVoltageComp(uint8_t vsCompMode);
    void setSwitchMode(uint8_t switchMode);
    void setOscMode(uint8_t oscillatorMode);
    void setAccKVAL(byte kvalInput);
    void setDecKVAL(byte kvalInput);
    void setRunKVAL(byte kvalInput);
    void setHoldKVAL(byte kvalInput);

    boolean getLoSpdOpt();
    // getSyncPin
    byte getStepMode();
    float getMaxSpeed();
    float getMinSpeed();
    float getFullSpeed();
    float getAcc();
    float getDec();
    byte getOCThreshold();
    uint8_t getPWMFreqDivisor();
    uint8_t getPWMFreqMultiplier();
    uint8_t getSlewRate();
    uint8_t getOCShutdown();
    uint8_t getVoltageComp();
    uint8_t getSwitchMode();
    uint8_t getOscMode();
    byte getAccKVAL();
    byte getDecKVAL();
    byte getRunKVAL();
    byte getHoldKVAL();
    
    // ...and now, operational commands.
    long getPos();
    long getMark();
    void run(byte dir, float stepsPerSec);
    void stepClock(byte dir);
    void move(byte dir, unsigned long numSteps);
    void goTo(long pos);
    void goToDir(byte dir, long pos);
    void goUntil(byte action, byte dir, float stepsPerSec);
    void releaseSw(byte action, byte dir);
    void goHome();
    void goMark();
    void setMark(long newMark);
    void setPos(long newPos);
    void resetPos();
    void resetDev();
    void softStop();
    void hardStop();
    void softHiZ();
    void hardHiZ();
    
    
  private:
    void SPIConfig();
    byte SPIXfer(byte data);
    unsigned long xferParam(unsigned long value, byte bitLen);
    unsigned long paramHandler(byte param, unsigned long value);
    
    // Support functions for converting from user units to L6470 units
    unsigned long accCalc(float stepsPerSecPerSec);
    unsigned long decCalc(float stepsPerSecPerSec);
    unsigned long minSpdCalc(float stepsPerSec);
    unsigned long maxSpdCalc(float stepsPerSec);
    unsigned long FSCalc(float stepsPerSec);
    unsigned long intSpdCalc(float stepsPerSec);
    unsigned long spdCalc(float stepsPerSec);

    // Support functions for converting from L6470 to user units
    float accParse(unsigned long stepsPerSecPerSec);
    float decParse(unsigned long stepsPerSecPerSec);
    float minSpdParse(unsigned long stepsPerSec);
    float maxSpdParse(unsigned long stepsPerSec);
    float FSParse(unsigned long stepsPerSec);
    float intSpdParse(unsigned long stepsPerSec);
    float spdParse(unsigned long stepsPerSec);
 
    uint8_t _CSPin;
    uint8_t _resetPin;
    uint8_t _busyPin;

};

// User constants for public functions.

// dSPIN direction options: functions that accept dir as an argument can be
//  passed one of these constants. These functions are:
//    run()
//    stepClock()
//    move()
//    goToDir()
//    goUntil()
//    releaseSw()
#define FWD  0x01
#define REV  0x00

// dSPIN action options: functions that accept action as an argument can be
//  passed one of these constants. The contents of ABSPOS will either be
//  reset or copied to MARK, depending on the value sent. These functions are:
//    goUntil()
//    releaseSw()
#define RESET_ABSPOS  0x00
#define COPY_ABSPOS   0x01

// configSyncPin() options: the !BUSY/SYNC pin can be configured to be low when
//  the chip is executing a command, *or* to output a pulse on each full step
//  clock (with some divisor). These 
#define BUSY_PIN   0x00     // !BUSY/SYNC pin set to !BUSY mode
#define SYNC_PIN   0x80     // pin set to SYNC mode

// divisors for SYNC pulse outputs
#define SYNC_FS_2  0x00   // two per full step
#define SYNC_FS    0x10   // one per full step
#define SYNC_2FS   0x20   // one per two full steps
#define SYNC_4FS   0x30   // one per four full steps
#define SYNC_8FS   0x40   // one per eight full steps
#define SYNC_16FS  0x50   // one per 16 full steps
#define SYNC_32FS  0x60   // one per 32 full steps
#define SYNC_64FS  0x70   // one per 64 full steps

// configStepMode() options: select the microsteps per full step.
#define STEP_FS    0x00   // one step per full step
#define STEP_FS_2  0x01   // two microsteps per full step
#define STEP_FS_4  0x02   // four microsteps per full step
#define STEP_FS_8  0x03   // etc.
#define STEP_FS_16 0x04
#define STEP_FS_32 0x05
#define STEP_FS_64 0x06
#define STEP_FS_128 0x07

// setOCThreshold() options
#define OC_375mA  0x00
#define OC_750mA  0x01
#define OC_1125mA 0x02
#define OC_1500mA 0x03
#define OC_1875mA 0x04
#define OC_2250mA 0x05
#define OC_2625mA 0x06
#define OC_3000mA 0x07
#define OC_3375mA 0x08
#define OC_3750mA 0x09
#define OC_4125mA 0x0A
#define OC_4500mA 0x0B
#define OC_4875mA 0x0C
#define OC_5250mA 0x0D
#define OC_5625mA 0x0E
#define OC_6000mA 0x0F

// PWM Multiplier and divisor options
#define PWM_MUL_0_625           (0x00)<<10
#define PWM_MUL_0_75            (0x01)<<10
#define PWM_MUL_0_875           (0x02)<<10
#define PWM_MUL_1               (0x03)<<10
#define PWM_MUL_1_25            (0x04)<<10
#define PWM_MUL_1_5             (0x05)<<10
#define PWM_MUL_1_75            (0x06)<<10
#define PWM_MUL_2               (0x07)<<10
#define PWM_DIV_1               (0x00)<<13
#define PWM_DIV_2               (0x01)<<13
#define PWM_DIV_3               (0x02)<<13
#define PWM_DIV_4               (0x03)<<13
#define PWM_DIV_5               (0x04)<<13
#define PWM_DIV_6               (0x05)<<13
#define PWM_DIV_7               (0x06)<<13

// Slew rate options
#define SR_180V_us              0x0000  // 180V/us
#define SR_290V_us              0x0200  // 290V/us
#define SR_530V_us              0x0300  // 530V/us

// Overcurrent bridge shutdown options
#define OC_SD_DISABLE           0x0000  // Bridges do NOT shutdown on OC detect
#define OC_SD_ENABLE            0x0080  // Bridges shutdown on OC detect

// Voltage compensation settings. See p 34 of datasheet.
#define VS_COMP_DISABLE         0x0000  // Disable motor voltage compensation.
#define VS_COMP_ENABLE          0x0020  // Enable motor voltage compensation.

// External switch input functionality.
#define SW_HARD_STOP            0x0000 // Default; hard stop motor on switch.
#define SW_USER                 0x0010 // Tie to the GoUntil and ReleaseSW
                                       //  commands to provide jog function.
                                       //  See page 25 of datasheet.
                                                   
// Clock functionality
#define INT_16MHZ               0x0000 // Internal 16MHz, no output
#define INT_16MHZ_OSCOUT_2MHZ   0x0008 // Default; internal 16MHz, 2MHz output
#define INT_16MHZ_OSCOUT_4MHZ   0x0009 // Internal 16MHz, 4MHz output
#define INT_16MHZ_OSCOUT_8MHZ   0x000A // Internal 16MHz, 8MHz output
#define INT_16MHZ_OSCOUT_16MHZ  0x000B // Internal 16MHz, 16MHz output
#define EXT_8MHZ_XTAL_DRIVE     0x0004 // External 8MHz crystal
#define EXT_16MHZ_XTAL_DRIVE    0x0005 // External 16MHz crystal
#define EXT_24MHZ_XTAL_DRIVE    0x0006 // External 24MHz crystal
#define EXT_32MHZ_XTAL_DRIVE    0x0007 // External 32MHz crystal
#define EXT_8MHZ_OSCOUT_INVERT  0x000C // External 8MHz crystal, output inverted
#define EXT_16MHZ_OSCOUT_INVERT 0x000D // External 16MHz crystal, output inverted
#define EXT_24MHZ_OSCOUT_INVERT 0x000E // External 24MHz crystal, output inverted
#define EXT_32MHZ_OSCOUT_INVERT 0x000F // External 32MHz crystal, output inverted 
#endif

