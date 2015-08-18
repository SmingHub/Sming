//dSPIN_commands.ino - Contains high-level command implementations- movement
//   and configuration commands, for example.

// Realize the "set parameter" function, to write to the various registers in
//  the dSPIN chip.
void dSPIN_SetParam(byte param, unsigned long value) 
{
  dSPIN_Xfer(dSPIN_SET_PARAM | param);
  dSPIN_ParamHandler(param, value);
}

// Realize the "get parameter" function, to read from the various registers in
//  the dSPIN chip.
unsigned long dSPIN_GetParam(byte param)
{
  dSPIN_Xfer(dSPIN_GET_PARAM | param);
  return dSPIN_ParamHandler(param, 0);
}

// Much of the functionality between "get parameter" and "set parameter" is
//  very similar, so we deal with that by putting all of it in one function
//  here to save memory space and simplify the program.
unsigned long dSPIN_ParamHandler(byte param, unsigned long value)
{
  unsigned long ret_val = 0;   // This is a temp for the value to return.
  // This switch structure handles the appropriate action for each register.
  //  This is necessary since not all registers are of the same length, either
  //  bit-wise or byte-wise, so we want to make sure we mask out any spurious
  //  bits and do the right number of transfers. That is handled by the dSPIN_Param()
  //  function, in most cases, but for 1-byte or smaller transfers, we call
  //  dSPIN_Xfer() directly.
  switch (param)
  {
    // ABS_POS is the current absolute offset from home. It is a 22 bit number expressed
    //  in two's complement. At power up, this value is 0. It cannot be written when
    //  the motor is running, but at any other time, it can be updated to change the
    //  interpreted position of the motor.
    case dSPIN_ABS_POS:
      ret_val = dSPIN_Param(value, 22);
      break;
    // EL_POS is the current electrical position in the step generation cycle. It can
    //  be set when the motor is not in motion. Value is 0 on power up.
    case dSPIN_EL_POS:
      ret_val = dSPIN_Param(value, 9);
      break;
    // MARK is a second position other than 0 that the motor can be told to go to. As
    //  with ABS_POS, it is 22-bit two's complement. Value is 0 on power up.
    case dSPIN_MARK:
      ret_val = dSPIN_Param(value, 22);
      break;
    // SPEED contains information about the current speed. It is read-only. It does 
    //  NOT provide direction information.
    case dSPIN_SPEED:
      ret_val = dSPIN_Param(0, 20);
      break; 
    // ACC and DEC set the acceleration and deceleration rates. Set ACC to 0xFFF 
    //  to get infinite acceleration/decelaeration- there is no way to get infinite
    //  deceleration w/o infinite acceleration (except the HARD STOP command).
    //  Cannot be written while motor is running. Both default to 0x08A on power up.
    // AccCalc() and DecCalc() functions exist to convert steps/s/s values into
    //  12-bit values for these two registers.
    case dSPIN_ACC: 
      ret_val = dSPIN_Param(value, 12);
      break;
    case dSPIN_DEC: 
      ret_val = dSPIN_Param(value, 12);
      break;
    // MAX_SPEED is just what it says- any command which attempts to set the speed
    //  of the motor above this value will simply cause the motor to turn at this
    //  speed. Value is 0x041 on power up.
    // MaxSpdCalc() function exists to convert steps/s value into a 10-bit value
    //  for this register.
    case dSPIN_MAX_SPEED:
      ret_val = dSPIN_Param(value, 10);
      break;
    // MIN_SPEED controls two things- the activation of the low-speed optimization
    //  feature and the lowest speed the motor will be allowed to operate at. LSPD_OPT
    //  is the 13th bit, and when it is set, the minimum allowed speed is automatically
    //  set to zero. This value is 0 on startup.
    // MinSpdCalc() function exists to convert steps/s value into a 12-bit value for this
    //  register. SetLSPDOpt() function exists to enable/disable the optimization feature.
    case dSPIN_MIN_SPEED: 
      ret_val = dSPIN_Param(value, 12);
      break;
    // FS_SPD register contains a threshold value above which microstepping is disabled
    //  and the dSPIN operates in full-step mode. Defaults to 0x027 on power up.
    // FSCalc() function exists to convert steps/s value into 10-bit integer for this
    //  register.
    case dSPIN_FS_SPD:
      ret_val = dSPIN_Param(value, 10);
      break;
    // KVAL is the maximum voltage of the PWM outputs. These 8-bit values are ratiometric
    //  representations: 255 for full output voltage, 128 for half, etc. Default is 0x29.
    // The implications of different KVAL settings is too complex to dig into here, but
    //  it will usually work to max the value for RUN, ACC, and DEC. Maxing the value for
    //  HOLD may result in excessive power dissipation when the motor is not running.
    case dSPIN_KVAL_HOLD:
      ret_val = dSPIN_Xfer((byte)value);
      break;
    case dSPIN_KVAL_RUN:
      ret_val = dSPIN_Xfer((byte)value);
      break;
    case dSPIN_KVAL_ACC:
      ret_val = dSPIN_Xfer((byte)value);
      break;
    case dSPIN_KVAL_DEC:
      ret_val = dSPIN_Xfer((byte)value);
      break;
    // INT_SPD, ST_SLP, FN_SLP_ACC and FN_SLP_DEC are all related to the back EMF
    //  compensation functionality. Please see the datasheet for details of this
    //  function- it is too complex to discuss here. Default values seem to work
    //  well enough.
    case dSPIN_INT_SPD:
      ret_val = dSPIN_Param(value, 14);
      break;
    case dSPIN_ST_SLP: 
      ret_val = dSPIN_Xfer((byte)value);
      break;
    case dSPIN_FN_SLP_ACC: 
      ret_val = dSPIN_Xfer((byte)value);
      break;
    case dSPIN_FN_SLP_DEC: 
      ret_val = dSPIN_Xfer((byte)value);
      break;
    // K_THERM is motor winding thermal drift compensation. Please see the datasheet
    //  for full details on operation- the default value should be okay for most users.
    case dSPIN_K_THERM: 
      ret_val = dSPIN_Xfer((byte)value & 0x0F);
      break;
    // ADC_OUT is a read-only register containing the result of the ADC measurements.
    //  This is less useful than it sounds; see the datasheet for more information.
    case dSPIN_ADC_OUT:
      ret_val = dSPIN_Xfer(0);
      break;
    // Set the overcurrent threshold. Ranges from 375mA to 6A in steps of 375mA.
    //  A set of defined constants is provided for the user's convenience. Default
    //  value is 3.375A- 0x08. This is a 4-bit value.
    case dSPIN_OCD_TH: 
      ret_val = dSPIN_Xfer((byte)value & 0x0F);
      break;
    // Stall current threshold. Defaults to 0x40, or 2.03A. Value is from 31.25mA to
    //  4A in 31.25mA steps. This is a 7-bit value.
    case dSPIN_STALL_TH: 
      ret_val = dSPIN_Xfer((byte)value & 0x7F);
      break;
    // STEP_MODE controls the microstepping settings, as well as the generation of an
    //  output signal from the dSPIN. Bits 2:0 control the number of microsteps per
    //  step the part will generate. Bit 7 controls whether the BUSY/SYNC pin outputs
    //  a BUSY signal or a step synchronization signal. Bits 6:4 control the frequency
    //  of the output signal relative to the full-step frequency; see datasheet for
    //  that relationship as it is too complex to reproduce here.
    // Most likely, only the microsteps per step value will be needed; there is a set
    //  of constants provided for ease of use of these values.
    case dSPIN_STEP_MODE:
      ret_val = dSPIN_Xfer((byte)value);
      break;
    // ALARM_EN controls which alarms will cause the FLAG pin to fall. A set of constants
    //  is provided to make this easy to interpret. By default, ALL alarms will trigger the
    //  FLAG pin.
    case dSPIN_ALARM_EN: 
      ret_val = dSPIN_Xfer((byte)value);
      break;
    // CONFIG contains some assorted configuration bits and fields. A fairly comprehensive
    //  set of reasonably self-explanatory constants is provided, but users should refer
    //  to the datasheet before modifying the contents of this register to be certain they
    //  understand the implications of their modifications. Value on boot is 0x2E88; this
    //  can be a useful way to verify proper start up and operation of the dSPIN chip.
    case dSPIN_CONFIG: 
      ret_val = dSPIN_Param(value, 16);
      break;
    // STATUS contains read-only information about the current condition of the chip. A
    //  comprehensive set of constants for masking and testing this register is provided, but
    //  users should refer to the datasheet to ensure that they fully understand each one of
    //  the bits in the register.
    case dSPIN_STATUS:  // STATUS is a read-only register
      ret_val = dSPIN_Param(0, 16);
      break;
    default:
      ret_val = dSPIN_Xfer((byte)(value));
      break;
  }
  return ret_val;
}

// Enable or disable the low-speed optimization option. If enabling,
//  the other 12 bits of the register will be automatically zero.
//  When disabling, the value will have to be explicitly written by
//  the user with a SetParam() call. See the datasheet for further
//  information about low-speed optimization.
void SetLSPDOpt(boolean enable)
{
  dSPIN_Xfer(dSPIN_SET_PARAM | dSPIN_MIN_SPEED);
  if (enable) dSPIN_Param(0x1000, 13);
  else dSPIN_Param(0, 13);
}
  
// RUN sets the motor spinning in a direction (defined by the constants
//  FWD and REV). Maximum speed and minimum speed are defined
//  by the MAX_SPEED and MIN_SPEED registers; exceeding the FS_SPD value
//  will switch the device into full-step mode.
// The SpdCalc() function is provided to convert steps/s values into
//  appropriate integer values for this function.
void dSPIN_Run(byte dir, unsigned long spd)
{
  dSPIN_Xfer(dSPIN_RUN | dir);
  if (spd > 0xFFFFF) spd = 0xFFFFF;
  dSPIN_Xfer((byte)(spd >> 16));
  dSPIN_Xfer((byte)(spd >> 8));
  dSPIN_Xfer((byte)(spd));
}

// STEP_CLOCK puts the device in external step clocking mode. When active,
//  pin 25, STCK, becomes the step clock for the device, and steps it in
//  the direction (set by the FWD and REV constants) imposed by the call
//  of this function. Motion commands (RUN, MOVE, etc) will cause the device
//  to exit step clocking mode.
void dSPIN_Step_Clock(byte dir)
{
  dSPIN_Xfer(dSPIN_STEP_CLOCK | dir);
}

// MOVE will send the motor n_step steps (size based on step mode) in the
//  direction imposed by dir (FWD or REV constants may be used). The motor
//  will accelerate according the acceleration and deceleration curves, and
//  will run at MAX_SPEED. Stepping mode will adhere to FS_SPD value, as well.
void dSPIN_Move(byte dir, unsigned long n_step)
{
  dSPIN_Xfer(dSPIN_MOVE | dir);
  if (n_step > 0x3FFFFF) n_step = 0x3FFFFF;
  dSPIN_Xfer((byte)(n_step >> 16));
  dSPIN_Xfer((byte)(n_step >> 8));
  dSPIN_Xfer((byte)(n_step));
}

// GOTO operates much like MOVE, except it produces absolute motion instead
//  of relative motion. The motor will be moved to the indicated position
//  in the shortest possible fashion.
void dSPIN_GoTo(unsigned long pos)
{
  
  dSPIN_Xfer(dSPIN_GOTO);
  if (pos > 0x3FFFFF) pos = 0x3FFFFF;
  dSPIN_Xfer((byte)(pos >> 16));
  dSPIN_Xfer((byte)(pos >> 8));
  dSPIN_Xfer((byte)(pos));
}

// Same as GOTO, but with user constrained rotational direction.
void dSPIN_GoTo_DIR(byte dir, unsigned long pos)
{
  
  dSPIN_Xfer(dSPIN_GOTO_DIR);
  if (pos > 0x3FFFFF) pos = 0x3FFFFF;
  dSPIN_Xfer((byte)(pos >> 16));
  dSPIN_Xfer((byte)(pos >> 8));
  dSPIN_Xfer((byte)(pos));
}

// GoUntil will set the motor running with direction dir (REV or
//  FWD) until a falling edge is detected on the SW pin. Depending
//  on bit SW_MODE in CONFIG, either a hard stop or a soft stop is
//  performed at the falling edge, and depending on the value of
//  act (either RESET or COPY) the value in the ABS_POS register is
//  either RESET to 0 or COPY-ed into the MARK register.
void dSPIN_GoUntil(byte act, byte dir, unsigned long spd)
{
  dSPIN_Xfer(dSPIN_GO_UNTIL | act | dir);
  if (spd > 0x3FFFFF) spd = 0x3FFFFF;
  dSPIN_Xfer((byte)(spd >> 16));
  dSPIN_Xfer((byte)(spd >> 8));
  dSPIN_Xfer((byte)(spd));
}

// Similar in nature to GoUntil, ReleaseSW produces motion at the
//  higher of two speeds: the value in MIN_SPEED or 5 steps/s.
//  The motor continues to run at this speed until a rising edge
//  is detected on the switch input, then a hard stop is performed
//  and the ABS_POS register is either COPY-ed into MARK or RESET to
//  0, depending on whether RESET or COPY was passed to the function
//  for act.
void dSPIN_ReleaseSW(byte act, byte dir)
{
  dSPIN_Xfer(dSPIN_RELEASE_SW | act | dir);
}

// GoHome is equivalent to GoTo(0), but requires less time to send.
//  Note that no direction is provided; motion occurs through shortest
//  path. If a direction is required, use GoTo_DIR().
void dSPIN_GoHome()
{
  dSPIN_Xfer(dSPIN_GO_HOME);
}

// GoMark is equivalent to GoTo(MARK), but requires less time to send.
//  Note that no direction is provided; motion occurs through shortest
//  path. If a direction is required, use GoTo_DIR().
void dSPIN_GoMark()
{
  dSPIN_Xfer(dSPIN_GO_MARK);
}

// Sets the ABS_POS register to 0, effectively declaring the current
//  position to be "HOME".
void dSPIN_ResetPos()
{
  dSPIN_Xfer(dSPIN_RESET_POS);
}

// Reset device to power up conditions. Equivalent to toggling the STBY
//  pin or cycling power.
void dSPIN_ResetDev()
{
  dSPIN_Xfer(dSPIN_RESET_DEVICE);
}
  
// Bring the motor to a halt using the deceleration curve.
void dSPIN_SoftStop()
{
  dSPIN_Xfer(dSPIN_SOFT_STOP);
}

// Stop the motor with infinite deceleration.
void dSPIN_HardStop()
{
  dSPIN_Xfer(dSPIN_HARD_STOP);
}

// Decelerate the motor and put the bridges in Hi-Z state.
void dSPIN_SoftHiZ()
{
  dSPIN_Xfer(dSPIN_SOFT_HIZ);
}

// Put the bridges in Hi-Z state immediately with no deceleration.
void dSPIN_HardHiZ()
{
  dSPIN_Xfer(dSPIN_HARD_HIZ);
}

// Fetch and return the 16-bit value in the STATUS register. Resets
//  any warning flags and exits any error states. Using GetParam()
//  to read STATUS does not clear these values.
int dSPIN_GetStatus()
{
  int temp = 0;
  dSPIN_Xfer(dSPIN_GET_STATUS);
  temp = dSPIN_Xfer(0)<<8;
  temp |= dSPIN_Xfer(0);
  return temp;
}

