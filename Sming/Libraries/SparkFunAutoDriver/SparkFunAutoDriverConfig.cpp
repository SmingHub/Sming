#include "SparkFunAutoDriver.h"

// Setup the SYNC/BUSY pin to be either SYNC or BUSY, and to a desired
//  ticks per step level.
void AutoDriver::configSyncPin(byte pinFunc, byte syncSteps)
{
  // Only some of the bits in this register are of interest to us; we need to
  //  clear those bits. It happens that they are the upper four.
  byte syncPinConfig = (byte)getParam(STEP_MODE);
  syncPinConfig &= 0x0F;
  
  // Now, let's OR in the arguments. We're going to mask the incoming
  //  data to avoid touching any bits that aren't appropriate. See datasheet
  //  for more info about which bits we're touching.
  syncPinConfig |= ((pinFunc & 0x80) | (syncSteps & 0x70));
  
  // Now we should be able to send that byte right back to the dSPIN- it
  //  won't corrupt the other bits, and the changes are made.
  setParam(STEP_MODE, (unsigned long)syncPinConfig);
}

// The dSPIN chip supports microstepping for a smoother ride. This function
//  provides an easy front end for changing the microstepping mode.
void AutoDriver::configStepMode(byte stepMode)
{

  // Only some of these bits are useful (the lower three). We'll extract the
  //  current contents, clear those three bits, then set them accordingly.
  byte stepModeConfig = (byte)getParam(STEP_MODE);
  stepModeConfig &= 0xF8;
  
  // Now we can OR in the new bit settings. Mask the argument so we don't
  //  accidentally the other bits, if the user sends us a non-legit value.
  stepModeConfig |= (stepMode&0x07);
  
  // Now push the change to the chip.
  setParam(STEP_MODE, (unsigned long)stepModeConfig);
}

byte AutoDriver::getStepMode() {
  return (byte)(getParam(STEP_MODE) & 0x07);
}

// This is the maximum speed the dSPIN will attempt to produce.
void AutoDriver::setMaxSpeed(float stepsPerSecond)
{
  // We need to convert the floating point stepsPerSecond into a value that
  //  the dSPIN can understand. Fortunately, we have a function to do that.
  unsigned long integerSpeed = maxSpdCalc(stepsPerSecond);
  
  // Now, we can set that paramter.
  setParam(MAX_SPEED, integerSpeed);
}


float AutoDriver::getMaxSpeed()
{
  return maxSpdParse(getParam(MAX_SPEED));
}

// Set the minimum speed allowable in the system. This is the speed a motion
//  starts with; it will then ramp up to the designated speed or the max
//  speed, using the acceleration profile.
void AutoDriver::setMinSpeed(float stepsPerSecond)
{
  // We need to convert the floating point stepsPerSecond into a value that
  //  the dSPIN can understand. Fortunately, we have a function to do that.
  unsigned long integerSpeed = minSpdCalc(stepsPerSecond);
  
  // MIN_SPEED also contains the LSPD_OPT flag, so we need to protect that.
  unsigned long temp = getParam(MIN_SPEED) & 0x00001000;
  
  // Now, we can set that paramter.
  setParam(MIN_SPEED, integerSpeed | temp);
}

float AutoDriver::getMinSpeed()
{
  return minSpdParse(getParam(MIN_SPEED));
}

// Above this threshold, the dSPIN will cease microstepping and go to full-step
//  mode. 
void AutoDriver::setFullSpeed(float stepsPerSecond)
{
  unsigned long integerSpeed = FSCalc(stepsPerSecond);
  setParam(FS_SPD, integerSpeed);
}

float AutoDriver::getFullSpeed()
{
  return FSParse(getParam(FS_SPD));
}

// Set the acceleration rate, in steps per second per second. This value is
//  converted to a dSPIN friendly value. Any value larger than 29802 will
//  disable acceleration, putting the chip in "infinite" acceleration mode.
void AutoDriver::setAcc(float stepsPerSecondPerSecond)
{
  unsigned long integerAcc = accCalc(stepsPerSecondPerSecond);
  setParam(ACC, integerAcc);
}

float AutoDriver::getAcc()
{
  return accParse(getParam(ACC));
}

// Same rules as setAcc().
void AutoDriver::setDec(float stepsPerSecondPerSecond)
{
  unsigned long integerDec = decCalc(stepsPerSecondPerSecond);
  setParam(DECEL, integerDec);
}

float AutoDriver::getDec()
{
  return accParse(getParam(DECEL));
}

void AutoDriver::setOCThreshold(byte threshold)
{
  setParam(OCD_TH, 0x0F & threshold);
}

byte AutoDriver::getOCThreshold()
{
  return (byte) (getParam(OCD_TH) & 0xF);
}

// The next few functions are all breakouts for individual options within the
//  single register CONFIG. We'll read CONFIG, blank some bits, then OR in the
//  new value.

// This is a multiplier/divider setup for the PWM frequency when microstepping.
//  Divisors of 1-7 are available; multipliers of .625-2 are available. See
//  datasheet for more details; it's not clear what the frequency being
//  multiplied/divided here is, but it is clearly *not* the actual clock freq.
void AutoDriver::setPWMFreq(int divisor, int multiplier)
{
  unsigned long configVal = getParam(CONFIG);
  
  // The divisor is set by config 15:13, so mask 0xE000 to clear them.
  configVal &= ~(0xE000);
  // The multiplier is set by config 12:10; mask is 0x1C00
  configVal &= ~(0x1C00);
  // Now we can OR in the masked-out versions of the values passed in.
  configVal |= ((0xE000&divisor)|(0x1C00&multiplier));
  setParam(CONFIG, configVal);
}

int AutoDriver::getPWMFreqDivisor()
{
  return (int) (getParam(CONFIG) & 0xE000);
}

int AutoDriver::getPWMFreqMultiplier()
{
  return (int) (getParam(CONFIG) & 0x1C00);
}

// Slew rate of the output in V/us. Can be 180, 290, or 530.
void AutoDriver::setSlewRate(int slewRate)
{
  unsigned long configVal = getParam(CONFIG);
  
  // These bits live in CONFIG 9:8, so the mask is 0x0300.
  configVal &= ~(0x0300);
  //Now, OR in the masked incoming value.
  configVal |= (0x0300&slewRate);
  setParam(CONFIG, configVal);
}

int AutoDriver::getSlewRate()
{
  return (int) (getParam(CONFIG) & 0x0300);
}

// Single bit- do we shutdown the drivers on overcurrent or not?
void AutoDriver::setOCShutdown(int OCShutdown)
{
  unsigned long configVal = getParam(CONFIG);
  // This bit is CONFIG 7, mask is 0x0080
  configVal &= ~(0x0080);
  //Now, OR in the masked incoming value.
  configVal |= (0x0080&OCShutdown);
  setParam(CONFIG, configVal);
}

int AutoDriver::getOCShutdown()
{
  return (int) (getParam(CONFIG) & 0x0080);
}

// Enable motor voltage compensation? Not at all straightforward- check out
//  p34 of the datasheet.
void AutoDriver::setVoltageComp(int vsCompMode)
{
  unsigned long configVal = getParam(CONFIG);
  // This bit is CONFIG 5, mask is 0x0020
  configVal &= ~(0x0020);
  //Now, OR in the masked incoming value.
  configVal |= (0x0020&vsCompMode);
  setParam(CONFIG, configVal);
}

int AutoDriver::getVoltageComp()
{
  return (int) (getParam(CONFIG) & 0x0020);
}

// The switch input can either hard-stop the driver _or_ activate an interrupt.
//  This bit allows you to select what it does.
void AutoDriver::setSwitchMode(int switchMode)
{
  unsigned long configVal = getParam(CONFIG);
  // This bit is CONFIG 4, mask is 0x0010
  configVal &= ~(0x0100);
  //Now, OR in the masked incoming value.
  configVal |= (0x0100&switchMode);
  setParam(CONFIG, configVal);
}

int AutoDriver::getSwitchMode()
{
  return (int) (getParam(CONFIG) & 0x0100);
}

// There are a number of clock options for this chip- it can be configured to
//  accept a clock, drive a crystal or resonator, and pass or not pass the
//  clock signal downstream. Theoretically, you can use pretty much any
//  frequency you want to drive it; practically, this library assumes it's
//  being driven at 16MHz. Also, the device will use these bits to set the
//  math used to figure out steps per second and stuff like that.
void AutoDriver::setOscMode(int oscillatorMode)
{
  unsigned long configVal = getParam(CONFIG);
  // These bits are CONFIG 3:0, mask is 0x000F
  configVal &= ~(0x000F);
  //Now, OR in the masked incoming value.
  configVal |= (0x000F&oscillatorMode);
  setParam(CONFIG, configVal);
}

int AutoDriver::getOscMode()
{
  return (int) (getParam(CONFIG) & 0x000F);
}

// The KVAL registers are...weird. I don't entirely understand how they differ
//  from the microstepping, but if you have trouble getting the motor to run,
//  tweaking KVAL has proven effective in the past. There's a separate register
//  for each case: running, static, accelerating, and decelerating.

void AutoDriver::setAccKVAL(byte kvalInput)
{
  setParam(KVAL_ACC, kvalInput);
}

byte AutoDriver::getAccKVAL()
{
  return (byte) getParam(KVAL_ACC);
}

void AutoDriver::setDecKVAL(byte kvalInput)
{
  setParam(KVAL_DEC, kvalInput);
}

byte AutoDriver::getDecKVAL()
{
  return (byte) getParam(KVAL_DEC);
}

void AutoDriver::setRunKVAL(byte kvalInput)
{
  setParam(KVAL_RUN, kvalInput);
}

byte AutoDriver::getRunKVAL()
{
  return (byte) getParam(KVAL_RUN);
}

void AutoDriver::setHoldKVAL(byte kvalInput)
{
  setParam(KVAL_HOLD, kvalInput);
}

byte AutoDriver::getHoldKVAL()
{
  return (byte) getParam(KVAL_HOLD);
}

// Enable or disable the low-speed optimization option. With LSPD_OPT enabled,
//  motion starts from 0 instead of MIN_SPEED and low-speed optimization keeps
//  the driving sine wave prettier than normal until MIN_SPEED is reached.
void AutoDriver::setLoSpdOpt(boolean enable)
{
  unsigned long temp = getParam(MIN_SPEED);
  if (enable) temp |= 0x00001000; // Set the LSPD_OPT bit
  else        temp &= 0xffffefff; // Clear the LSPD_OPT bit
  setParam(MIN_SPEED, temp);
}

boolean AutoDriver::getLoSpdOpt()
{
  return (boolean) ((getParam(MIN_SPEED) & 0x00001000) != 0);
}
