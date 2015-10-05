#include "SparkFunAutoDriver.h"

//commands.ino - Contains high-level command implementations- movement
//   and configuration commands, for example.

// Realize the "set parameter" function, to write to the various registers in
//  the dSPIN chip.
void AutoDriver::setParam(byte param, unsigned long value) 
{
  param |= SET_PARAM;
  SPIXfer((byte)param);
  paramHandler(param, value);
}

// Realize the "get parameter" function, to read from the various registers in
//  the dSPIN chip.
unsigned long AutoDriver::getParam(byte param)
{
  SPIXfer(param | GET_PARAM);
  return paramHandler(param, 0);
}

// Returns the content of the ABS_POS register, which is a signed 22-bit number
//  indicating the number of steps the motor has traveled from the HOME
//  position. HOME is defined by zeroing this register, and it is zero on
//  startup.
long AutoDriver::getPos()
{
  long temp = getParam(ABS_POS);
  
  // Since ABS_POS is a 22-bit 2's comp value, we need to check bit 21 and, if
  //  it's set, set all the bits ABOVE 21 in order for the value to maintain
  //  its appropriate sign.
  if (temp & 0x00200000) temp |= 0xffC00000;
  return temp;
}

// Just like getPos(), but for MARK.
long AutoDriver::getMark()
{
  long temp = getParam(MARK);
  
  // Since ABS_POS is a 22-bit 2's comp value, we need to check bit 21 and, if
  //  it's set, set all the bits ABOVE 21 in order for the value to maintain
  //  its appropriate sign.
  if (temp & 0x00200000) temp |= 0xffC00000;
  return temp;
}

// RUN sets the motor spinning in a direction (defined by the constants
//  FWD and REV). Maximum speed and minimum speed are defined
//  by the MAX_SPEED and MIN_SPEED registers; exceeding the FS_SPD value
//  will switch the device into full-step mode.
// The spdCalc() function is provided to convert steps/s values into
//  appropriate integer values for this function.
void AutoDriver::run(byte dir, float stepsPerSec)
{
  SPIXfer(RUN | dir);
  unsigned long integerSpeed = spdCalc(stepsPerSec);
  if (integerSpeed > 0xFFFFF) integerSpeed = 0xFFFFF;
  
  // Now we need to push this value out to the dSPIN. The 32-bit value is
  //  stored in memory in little-endian format, but the dSPIN expects a
  //  big-endian output, so we need to reverse the byte-order of the
  //  data as we're sending it out. Note that only 3 of the 4 bytes are
  //  valid here.
  
  // We begin by pointing bytePointer at the first byte in integerSpeed.
  byte* bytePointer = (byte*)&integerSpeed;
  // Next, we'll iterate through a for loop, indexing across the bytes in
  //  integerSpeed starting with byte 2 and ending with byte 0.
  for (char i = 2; i >= 0; i--)
  {
    SPIXfer(bytePointer[i]);
  }
}

// STEP_CLOCK puts the device in external step clocking mode. When active,
//  pin 25, STCK, becomes the step clock for the device, and steps it in
//  the direction (set by the FWD and REV constants) imposed by the call
//  of this function. Motion commands (RUN, MOVE, etc) will cause the device
//  to exit step clocking mode.
void AutoDriver::stepClock(byte dir)
{
  SPIXfer(STEP_CLOCK | dir);
}

// MOVE will send the motor numStep full steps in the
//  direction imposed by dir (FWD or REV constants may be used). The motor
//  will accelerate according the acceleration and deceleration curves, and
//  will run at MAX_SPEED. Stepping mode will adhere to FS_SPD value, as well.
void AutoDriver::move(byte dir, unsigned long numSteps)
{
  SPIXfer(MOVE | dir);
  if (numSteps > 0x3FFFFF) numSteps = 0x3FFFFF;
  // See run() for an explanation of what's going on here.
  byte* bytePointer = (byte*)&numSteps;
  for (char i = 2; i >= 0; i--)
  {
    SPIXfer(bytePointer[i]);
  }
}

// GOTO operates much like MOVE, except it produces absolute motion instead
//  of relative motion. The motor will be moved to the indicated position
//  in the shortest possible fashion.
void AutoDriver::goTo(long pos)
{
  SPIXfer(GOTO);
  if (pos > 0x3FFFFF) pos = 0x3FFFFF;
  // See run() for an explanation of what's going on here.
  byte* bytePointer = (byte*)&pos;
  for (char i = 2; i >= 0; i--)
  {
    SPIXfer(bytePointer[i]);
  }
}

// Same as GOTO, but with user constrained rotational direction.
void AutoDriver::goToDir(byte dir, long pos)
{
  SPIXfer(GOTO_DIR | dir);
  if (pos > 0x3FFFFF) pos = 0x3FFFFF;
  // See run() for an explanation of what's going on here.
  byte* bytePointer = (byte*)&pos;
  for (char i = 2; i >= 0; i--)
  {
    SPIXfer(bytePointer[i]);
  }
}

// GoUntil will set the motor running with direction dir (REV or
//  FWD) until a falling edge is detected on the SW pin. Depending
//  on bit SW_MODE in CONFIG, either a hard stop or a soft stop is
//  performed at the falling edge, and depending on the value of
//  act (either RESET or COPY) the value in the ABS_POS register is
//  either RESET to 0 or COPY-ed into the MARK register.
void AutoDriver::goUntil(byte action, byte dir, float stepsPerSec)
{
  SPIXfer(GO_UNTIL | action | dir);
  unsigned long integerSpeed = spdCalc(stepsPerSec);
  if (integerSpeed > 0x3FFFFF) integerSpeed = 0x3FFFFF;
  // See run() for an explanation of what's going on here.
  byte* bytePointer = (byte*)&integerSpeed;
  for (char i = 2; i >= 0; i--)
  {
    SPIXfer(bytePointer[i]);
  }
}

// Similar in nature to GoUntil, ReleaseSW produces motion at the
//  higher of two speeds: the value in MIN_SPEED or 5 steps/s.
//  The motor continues to run at this speed until a rising edge
//  is detected on the switch input, then a hard stop is performed
//  and the ABS_POS register is either COPY-ed into MARK or RESET to
//  0, depending on whether RESET or COPY was passed to the function
//  for act.
void AutoDriver::releaseSw(byte action, byte dir)
{
  SPIXfer(RELEASE_SW | action | dir);
}

// GoHome is equivalent to GoTo(0), but requires less time to send.
//  Note that no direction is provided; motion occurs through shortest
//  path. If a direction is required, use GoTo_DIR().
void AutoDriver::goHome()
{
  SPIXfer(GO_HOME);
}

// GoMark is equivalent to GoTo(MARK), but requires less time to send.
//  Note that no direction is provided; motion occurs through shortest
//  path. If a direction is required, use GoTo_DIR().
void AutoDriver::goMark()
{
  SPIXfer(GO_MARK);
}

// setMark() and setHome() allow the user to define new MARK or
//  ABS_POS values.
void AutoDriver::setMark(long newMark)
{
  setParam(MARK, newMark);
}

void AutoDriver::setPos(long newPos)
{
  setParam(ABS_POS, newPos);
}

// Sets the ABS_POS register to 0, effectively declaring the current
//  position to be "HOME".
void AutoDriver::resetPos()
{
  SPIXfer(RESET_POS);
}

// Reset device to power up conditions. Equivalent to toggling the STBY
//  pin or cycling power.
void AutoDriver::resetDev()
{
  SPIXfer(RESET_DEVICE);
}
  
// Bring the motor to a halt using the deceleration curve.
void AutoDriver::softStop()
{
  SPIXfer(SOFT_STOP);
}

// Stop the motor with infinite deceleration.
void AutoDriver::hardStop()
{
  SPIXfer(HARD_STOP);
}

// Decelerate the motor and put the bridges in Hi-Z state.
void AutoDriver::softHiZ()
{
  SPIXfer(SOFT_HIZ);
}

// Put the bridges in Hi-Z state immediately with no deceleration.
void AutoDriver::hardHiZ()
{
  SPIXfer(HARD_HIZ);
}

// Fetch and return the 16-bit value in the STATUS register. Resets
//  any warning flags and exits any error states. Using GetParam()
//  to read STATUS does not clear these values.
int AutoDriver::getStatus()
{
  int temp = 0;
  byte* bytePointer = (byte*)&temp;
  SPIXfer(GET_STATUS);
  bytePointer[1] = SPIXfer(0);
  bytePointer[0] = SPIXfer(0);
  return temp;
}
