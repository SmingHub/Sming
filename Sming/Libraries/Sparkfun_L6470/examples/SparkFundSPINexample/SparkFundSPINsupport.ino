//dSPIN_support.ino - Contains functions used to implement the high-level commands,
//   as well as utility functions for converting real-world units (eg, steps/s) to
//   values usable by the dsPIN controller. Also contains the specialized configuration
//   function for the dsPIN chip and the onboard peripherals needed to use it.

// The value in the ACC register is [(steps/s/s)*(tick^2)]/(2^-40) where tick is 
//  250ns (datasheet value)- 0x08A on boot.
// Multiply desired steps/s/s by .137438 to get an appropriate value for this register.
// This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
unsigned long AccCalc(float stepsPerSecPerSec)
{
  float temp = stepsPerSecPerSec * 0.137438;
  if( (unsigned long) long(temp) > 0x00000FFF) return 0x00000FFF;
  else return (unsigned long) long(temp);
}

// The calculation for DEC is the same as for ACC. Value is 0x08A on boot.
// This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
unsigned long DecCalc(float stepsPerSecPerSec)
{
  float temp = stepsPerSecPerSec * 0.137438;
  if( (unsigned long) long(temp) > 0x00000FFF) return 0x00000FFF;
  else return (unsigned long) long(temp);
}

// The value in the MAX_SPD register is [(steps/s)*(tick)]/(2^-18) where tick is 
//  250ns (datasheet value)- 0x041 on boot.
// Multiply desired steps/s by .065536 to get an appropriate value for this register
// This is a 10-bit value, so we need to make sure it remains at or below 0x3FF
unsigned long MaxSpdCalc(float stepsPerSec)
{
  float temp = stepsPerSec * .065536;
  if( (unsigned long) long(temp) > 0x000003FF) return 0x000003FF;
  else return (unsigned long) long(temp);
}

// The value in the MIN_SPD register is [(steps/s)*(tick)]/(2^-24) where tick is 
//  250ns (datasheet value)- 0x000 on boot.
// Multiply desired steps/s by 4.1943 to get an appropriate value for this register
// This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
unsigned long MinSpdCalc(float stepsPerSec)
{
  float temp = stepsPerSec * 4.1943;
  if( (unsigned long) long(temp) > 0x00000FFF) return 0x00000FFF;
  else return (unsigned long) long(temp);
}

// The value in the FS_SPD register is ([(steps/s)*(tick)]/(2^-18))-0.5 where tick is 
//  250ns (datasheet value)- 0x027 on boot.
// Multiply desired steps/s by .065536 and subtract .5 to get an appropriate value for this register
// This is a 10-bit value, so we need to make sure the value is at or below 0x3FF.
unsigned long FSCalc(float stepsPerSec)
{
  float temp = (stepsPerSec * .065536)-.5;
  if( (unsigned long) long(temp) > 0x000003FF) return 0x000003FF;
  else return (unsigned long) long(temp);
}

// The value in the INT_SPD register is [(steps/s)*(tick)]/(2^-24) where tick is 
//  250ns (datasheet value)- 0x408 on boot.
// Multiply desired steps/s by 4.1943 to get an appropriate value for this register
// This is a 14-bit value, so we need to make sure the value is at or below 0x3FFF.
unsigned long IntSpdCalc(float stepsPerSec)
{
  float temp = stepsPerSec * 4.1943;
  if( (unsigned long) long(temp) > 0x00003FFF) return 0x00003FFF;
  else return (unsigned long) long(temp);
}

// When issuing RUN command, the 20-bit speed is [(steps/s)*(tick)]/(2^-28) where tick is 
//  250ns (datasheet value).
// Multiply desired steps/s by 67.106 to get an appropriate value for this register
// This is a 20-bit value, so we need to make sure the value is at or below 0xFFFFF.
unsigned long SpdCalc(float stepsPerSec)
{
  float temp = stepsPerSec * 67.106;
  if( (unsigned long) long(temp) > 0x000FFFFF) return 0x000FFFFF;
  else return (unsigned long)temp;
}

// Generalization of the subsections of the register read/write functionality.
//  We want the end user to just write the value without worrying about length,
//  so we pass a bit length parameter from the calling function.
unsigned long dSPIN_Param(unsigned long value, byte bit_len)
{
  unsigned long ret_val=0;        // We'll return this to generalize this function
                                  //  for both read and write of registers.
  byte byte_len = bit_len/8;      // How many BYTES do we have?
  if (bit_len%8 > 0) byte_len++;  // Make sure not to lose any partial byte values.
  // Let's make sure our value has no spurious bits set, and if the value was too
  //  high, max it out.
  unsigned long mask = 0xffffffff >> (32-bit_len);
  if (value > mask) value = mask;
  // The following three if statements handle the various possible byte length
  //  transfers- it'll be no less than 1 but no more than 3 bytes of data.
  // dSPIN_Xfer() sends a byte out through SPI and returns a byte received
  //  over SPI- when calling it, we typecast a shifted version of the masked
  //  value, then we shift the received value back by the same amount and
  //  store it until return time.
  if (byte_len == 3) {
    ret_val |= dSPIN_Xfer((byte)(value>>16)) << 16;
    //Serial.println(ret_val, HEX);
  }
  if (byte_len >= 2) {
    ret_val |= dSPIN_Xfer((byte)(value>>8)) << 8;
    //Serial.println(ret_val, HEX);
  }
  if (byte_len >= 1) {
    ret_val |= dSPIN_Xfer((byte)value);
    //Serial.println(ret_val, HEX);
  }
  // Return the received values. Mask off any unnecessary bits, just for
  //  the sake of thoroughness- we don't EXPECT to see anything outside
  //  the bit length range but better to be safe than sorry.
  return (ret_val & mask);
}

// This simple function shifts a byte out over SPI and receives a byte over
//  SPI. Unusually for SPI devices, the dSPIN requires a toggling of the
//  CS (slaveSelect) pin after each byte sent. That makes this function
//  a bit more reasonable, because we can include more functionality in it.
byte dSPIN_Xfer(byte data)
{
  byte data_out;
  digitalWrite(SLAVE_SELECT_PIN,LOW);
  // SPI.transfer() both shifts a byte out on the MOSI pin AND receives a
  //  byte in on the MISO pin.
  data_out = SPI.transfer(data);
  digitalWrite(SLAVE_SELECT_PIN,HIGH);
  return data_out;
}

// This is the generic initialization function to set up the Arduino to
//  communicate with the dSPIN chip. 
void dSPIN_init()
{
  // set up the input/output pins for the application.
  pinMode(10, OUTPUT);  // The SPI peripheral REQUIRES the hardware SS pin-
                        //  pin 10- to be an output. This is in here just
                        //  in case some future user makes something other
                        //  than pin 10 the SS pin.
  pinMode(SLAVE_SELECT_PIN, OUTPUT);
  digitalWrite(SLAVE_SELECT_PIN, HIGH);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);
  pinMode(dSPIN_BUSYN, INPUT);
  pinMode(dSPIN_RESET, OUTPUT);
  
  // reset the dSPIN chip. This could also be accomplished by
  //  calling the "dSPIN_ResetDev()" function after SPI is initialized.
  digitalWrite(dSPIN_RESET, HIGH);
  delay(1);
  digitalWrite(dSPIN_RESET, LOW);
  delay(1);
  digitalWrite(dSPIN_RESET, HIGH);
  delay(1);
  
  // initialize SPI for the dSPIN chip's needs:
  //  most significant bit first,
  //  SPI clock not to exceed 5MHz,
  //  SPI_MODE3 (clock idle high, latch data on rising edge of clock)  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16); // or 2, 8, 16, 32, 64
  SPI.setDataMode(SPI_MODE3);
}
