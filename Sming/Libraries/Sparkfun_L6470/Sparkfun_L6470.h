/*********************************************************************
This is a library for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/


#if ARDUINO >= 100
 #include "Arduino.h"
 #define WIRE_WRITE Wire.write
#else
 #include "WProgram.h"
  #define WIRE_WRITE Wire.send
#endif

#include "../../SmingCore/SPI.h"
#define PCD8544_SPI_CLOCK_DIV SPI_CLOCK_DIV4

#ifdef __SAM3X8E__
 typedef volatile RwReg PortReg;
 typedef uint32_t PortMask;
#else
  typedef volatile uint8_t PortReg;
  typedef uint8_t PortMask;
#endif


class Sparkfun_L6470 {
 public:
  // Software SPI with explicit CS pin.
  Sparkfun_L6470(int8_t SCLK, int8_t DIN, int8_t DC, int8_t CS, int8_t RST);
  // Software SPI with CS tied to ground.  Saves a pin but other pins can't be shared with other hardware.
  Sparkfun_L6470(int8_t SCLK, int8_t DIN, int8_t DC, int8_t RST);
  // Hardware SPI based on hardware controlled SCK (SCLK) and MOSI (DIN) pins. CS is still controlled by any IO pin.
  // NOTE: MISO and SS will be set as an input and output respectively, so be careful sharing those pins!
  Sparkfun_L6470(int8_t DC, int8_t CS, int8_t RST);

  void begin(uint8_t contrast = 40, uint8_t bias = 0x04);
  
  void command(uint8_t c);
  void data(uint8_t c);

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
  int8_t _din, _sclk, _dc, _rst, _cs;
  volatile PortReg  *mosiport, *clkport;
  PortMask mosipinmask, clkpinmask;

  void spiWrite(uint8_t c);
  bool isHardwareSPI();

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

};
