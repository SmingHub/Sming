/*----------------------------------------------------------------------*
 * DS3232RTC.cpp - Arduino library for the Maxim Integrated DS3232      *
 * Real-Time Clock. This library is intended for use with the Arduino   *
 * Time.h library, http://www.arduino.cc/playground/Code/Time           *
 *                                                                      *
 * This library is a drop-in replacement for the DS1307RTC.h library    *
 * by Michael Margolis that is supplied with the Arduino Time library   *
 * above. To change from using a DS1307 RTC to an DS3232 RTC, it is     *
 * only necessary to change the #include statement to include this      *
 * library instead of DS1307RTC.h.                                      *
 *                                                                      *
 * This library is *not* a drop-in replacement for the newer version    *
 * of the DS1307RTC library at                                          *
 * http://www.pjrc.com/teensy/td_libs_DS1307RTC.html                    *
 *                                                                      *
 * In addition, this library implements functions to support the        *
 * additional features of the DS3232.                                   *
 *                                                                      *
 * This library will also work with the DS3231 RTC, which has the same  *
 * features of the DS3232 except: (1) Battery-backed SRAM, (2) Battery- *
 * backed 32kHz output (BB32kHz bit in Control/Status register 0x0F),   *
 * and (3) Adjustable temperature sensor sample rate (CRATE1:0 bits in  *
 * the Control/Status register).                                        *
 *                                                                      *
 * Whether used with the DS3232 or DS3231, the user is responsible for  *
 * ensuring reads and writes do not exceed the device's address space   *
 * (0x00-0x12 for DS3231, 0x00-0xFF for DS3232); no bounds checking     *
 * is done by this library.                                             *
 *                                                                      *
 * Jack Christensen 06Mar2013                                           *
 * http://github.com/JChristensen/DS3232RTC                             *
 *                                                                      *
 * CC BY-SA 4.0                                                         *
 * "Arduino DS3232RTC Library" by Jack Christensen is licensed under    *
 * CC BY-SA 4.0, http://creativecommons.org/licenses/by-sa/4.0/         *
 *----------------------------------------------------------------------*/ 

#include "DS3232RTC.h"

//define release-independent I2C functions
#if defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#include <TinyWireM.h>
#define i2cBegin TinyWireM.begin
#define i2cBeginTransmission TinyWireM.beginTransmission
#define i2cEndTransmission TinyWireM.endTransmission
#define i2cRequestFrom TinyWireM.requestFrom
#define i2cRead TinyWireM.receive
#define i2cWrite TinyWireM.send
#elif ARDUINO >= 100
#include <Wire.h>
#define i2cBegin Wire.begin
#define i2cBeginTransmission Wire.beginTransmission
#define i2cEndTransmission Wire.endTransmission
#define i2cRequestFrom Wire.requestFrom
#define i2cRead Wire.read
#define i2cWrite Wire.write
#else
#include <Wire.h>
#define i2cBegin Wire.begin
#define i2cBeginTransmission Wire.beginTransmission
#define i2cEndTransmission Wire.endTransmission
#define i2cRequestFrom Wire.requestFrom
#define i2cRead Wire.receive
#define i2cWrite Wire.send
#endif

/*============================================================================*/	
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
 
void breakTime(time_t timeInput, tmElements_t &tm){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  tm.Second = time % 60;
  time /= 60; // now it is minutes
  tm.Minute = time % 60;
  time /= 60; // now it is hours
  tm.Hour = time % 24;
  time /= 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1 
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.Year = year; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm.Month = month + 1;  // jan is month 1  
  tm.Day = time + 1;     // day of month
}

time_t makeTime(tmElements_t &tm){   
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm.Year*(SECS_PER_DAY * 365);
  for (i = 0; i < tm.Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < tm.Month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm.Year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm.Day-1) * SECS_PER_DAY;
  seconds+= tm.Hour * SECS_PER_HOUR;
  seconds+= tm.Minute * SECS_PER_MIN;
  seconds+= tm.Second;
  return (time_t)seconds; 
}
/*=====================================================*/	
/* Low level system time functions  */

/*----------------------------------------------------------------------*
 * Constructor.                                                         *
 *----------------------------------------------------------------------*/
DS3232RTC::DS3232RTC()
{
}
  
/*----------------------------------------------------------------------*
 * Reads the current time from the RTC and returns it as a time_t       *
 * value. Returns a zero value if an I2C error occurred (e.g. RTC       *
 * not present).                                                        *
 *----------------------------------------------------------------------*/
time_t DS3232RTC::get()
{
    tmElements_t tm;
    
    if ( read(tm) ) return 0;
    return( makeTime(tm) );
}

/*----------------------------------------------------------------------*
 * Sets the RTC to the given time_t value and clears the                *
 * oscillator stop flag (OSF) in the Control/Status register.           *
 * Returns the I2C status (zero if successful).                         *
 *----------------------------------------------------------------------*/
byte DS3232RTC::set(time_t t)
{
    tmElements_t tm;

    breakTime(t, tm);
    return ( write(tm) );
}

/*----------------------------------------------------------------------*
 * Reads the current time from the RTC and returns it in a tmElements_t *
 * structure. Returns the I2C status (zero if successful).              *
 *----------------------------------------------------------------------*/
byte DS3232RTC::read(tmElements_t &tm)
{
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite((uint8_t)RTC_SECONDS);
    if ( byte e = i2cEndTransmission() ) return e;
    //request 7 bytes (secs, min, hr, dow, date, mth, yr)
    i2cRequestFrom(RTC_ADDR, tmNbrFields);
    tm.Second = bcd2dec(i2cRead() & ~_BV(DS1307_CH));   
    tm.Minute = bcd2dec(i2cRead());
    tm.Hour = bcd2dec(i2cRead() & ~_BV(HR1224));    //assumes 24hr clock
    tm.Wday = i2cRead();
    tm.Day = bcd2dec(i2cRead());
    tm.Month = bcd2dec(i2cRead() & ~_BV(CENTURY));  //don't use the Century bit
    tm.Year = y2kYearToTm(bcd2dec(i2cRead()));
    return 0;
}

/*----------------------------------------------------------------------*
 * Sets the RTC's time from a tmElements_t structure and clears the     *
 * oscillator stop flag (OSF) in the Control/Status register.           *
 * Returns the I2C status (zero if successful).                         *
 *----------------------------------------------------------------------*/
byte DS3232RTC::write(tmElements_t &tm)
{
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite((uint8_t)RTC_SECONDS);
    i2cWrite(dec2bcd(tm.Second));
    i2cWrite(dec2bcd(tm.Minute));
    i2cWrite(dec2bcd(tm.Hour));         //sets 24 hour format (Bit 6 == 0)
    i2cWrite(tm.Wday);
    i2cWrite(dec2bcd(tm.Day));
    i2cWrite(dec2bcd(tm.Month));
    i2cWrite(dec2bcd(tmYearToY2k(tm.Year))); 
    byte ret = i2cEndTransmission();
    uint8_t s = readRTC(RTC_STATUS);        //read the status register
    writeRTC( RTC_STATUS, s & ~_BV(OSF) );  //clear the Oscillator Stop Flag
    return ret;
}

/*----------------------------------------------------------------------*
 * Write multiple bytes to RTC RAM.                                     *
 * Valid address range is 0x00 - 0xFF, no checking.                     *
 * Number of bytes (nBytes) must be between 1 and 31 (Wire library      *
 * limitation).                                                         *
 * Returns the I2C status (zero if successful).                         *
 *----------------------------------------------------------------------*/
byte DS3232RTC::writeRTC(byte addr, byte *values, byte nBytes)
{
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite(addr);
    for (byte i=0; i<nBytes; i++) i2cWrite(values[i]);
    return i2cEndTransmission();
}

/*----------------------------------------------------------------------*
 * Write a single byte to RTC RAM.                                      *
 * Valid address range is 0x00 - 0xFF, no checking.                     *
 * Returns the I2C status (zero if successful).                         *
 *----------------------------------------------------------------------*/
byte DS3232RTC::writeRTC(byte addr, byte value)
{
    return ( writeRTC(addr, &value, 1) );
}

/*----------------------------------------------------------------------*
 * Read multiple bytes from RTC RAM.                                    *
 * Valid address range is 0x00 - 0xFF, no checking.                     *
 * Number of bytes (nBytes) must be between 1 and 32 (Wire library      *
 * limitation).                                                         *
 * Returns the I2C status (zero if successful).                         *
 *----------------------------------------------------------------------*/
byte DS3232RTC::readRTC(byte addr, byte *values, byte nBytes)
{
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite(addr);
    if ( byte e = i2cEndTransmission() ) return e;
    i2cRequestFrom( (uint8_t)RTC_ADDR, nBytes );
    for (byte i=0; i<nBytes; i++) values[i] = i2cRead();
    return 0;
}

/*----------------------------------------------------------------------*
 * Read a single byte from RTC RAM.                                     *
 * Valid address range is 0x00 - 0xFF, no checking.                     *
 *----------------------------------------------------------------------*/
byte DS3232RTC::readRTC(byte addr)
{
    byte b;
    
    readRTC(addr, &b, 1);
    return b;
}

/*----------------------------------------------------------------------*
 * Set an alarm time. Sets the alarm registers only.  To cause the      *
 * INT pin to be asserted on alarm match, use alarmInterrupt().         *
 * This method can set either Alarm 1 or Alarm 2, depending on the      *
 * value of alarmType (use a value from the ALARM_TYPES_t enumeration). *
 * When setting Alarm 2, the seconds value must be supplied but is      *
 * ignored, recommend using zero. (Alarm 2 has no seconds register.)    *
 *----------------------------------------------------------------------*/
void DS3232RTC::setAlarm(ALARM_TYPES_t alarmType, byte seconds, byte minutes, byte hours, byte daydate)
{
    uint8_t addr;
    
    seconds = dec2bcd(seconds);
    minutes = dec2bcd(minutes);
    hours = dec2bcd(hours);
    daydate = dec2bcd(daydate);
    if (alarmType & 0x01) seconds |= _BV(A1M1);
    if (alarmType & 0x02) minutes |= _BV(A1M2);
    if (alarmType & 0x04) hours |= _BV(A1M3);
    if (alarmType & 0x10) hours |= _BV(DYDT);
    if (alarmType & 0x08) daydate |= _BV(A1M4);
    
    if ( !(alarmType & 0x80) ) {    //alarm 1
        addr = ALM1_SECONDS;
        writeRTC(addr++, seconds);
    }
    else {
        addr = ALM2_MINUTES;
    }
    writeRTC(addr++, minutes);
    writeRTC(addr++, hours);
    writeRTC(addr++, daydate);
}

/*----------------------------------------------------------------------*
 * Set an alarm time. Sets the alarm registers only.  To cause the      *
 * INT pin to be asserted on alarm match, use alarmInterrupt().         *
 * This method can set either Alarm 1 or Alarm 2, depending on the      *
 * value of alarmType (use a value from the ALARM_TYPES_t enumeration). *
 * However, when using this method to set Alarm 1, the seconds value    *
 * is set to zero. (Alarm 2 has no seconds register.)                   *
 *----------------------------------------------------------------------*/
void DS3232RTC::setAlarm(ALARM_TYPES_t alarmType, byte minutes, byte hours, byte daydate)
{
    setAlarm(alarmType, 0, minutes, hours, daydate);
}

/*----------------------------------------------------------------------*
 * Enable or disable an alarm "interrupt" which asserts the INT pin     *
 * on the RTC.                                                          *
 *----------------------------------------------------------------------*/
void DS3232RTC::alarmInterrupt(byte alarmNumber, bool interruptEnabled)
{
    uint8_t controlReg, mask;
    
    controlReg = readRTC(RTC_CONTROL);
    mask = _BV(A1IE) << (alarmNumber - 1);
    if (interruptEnabled)
        controlReg |= mask;
    else
        controlReg &= ~mask;
    writeRTC(RTC_CONTROL, controlReg); 
}

/*----------------------------------------------------------------------*
 * Returns true or false depending on whether the given alarm has been  *
 * triggered, and resets the alarm flag bit.                            *
 *----------------------------------------------------------------------*/
bool DS3232RTC::alarm(byte alarmNumber)
{
    uint8_t statusReg, mask;
    
    statusReg = readRTC(RTC_STATUS);
    mask = _BV(A1F) << (alarmNumber - 1);
    if (statusReg & mask) {
        statusReg &= ~mask;
        writeRTC(RTC_STATUS, statusReg);
        return true;
    }
    else {
        return false;
    }
}

/*----------------------------------------------------------------------*
 * Enable or disable the square wave output.                            *
 * Use a value from the SQWAVE_FREQS_t enumeration for the parameter.   *
 *----------------------------------------------------------------------*/
void DS3232RTC::squareWave(SQWAVE_FREQS_t freq)
{
    uint8_t controlReg;

    controlReg = readRTC(RTC_CONTROL);
    if (freq >= SQWAVE_NONE) {
        controlReg |= _BV(INTCN);
    }
    else {
        controlReg = (controlReg & 0xE3) | (freq << RS1);
    }
    writeRTC(RTC_CONTROL, controlReg);
}

/*----------------------------------------------------------------------*
 * Returns the value of the oscillator stop flag (OSF) bit in the       *
 * control/status register which indicates that the oscillator is or    *
 * was stopped, and that the timekeeping data may be invalid.           *
 * Optionally clears the OSF bit depending on the argument passed.      *
 *----------------------------------------------------------------------*/
bool DS3232RTC::oscStopped(bool clearOSF)
{
    uint8_t s = readRTC(RTC_STATUS);    //read the status register
    bool ret = s & _BV(OSF);            //isolate the osc stop flag to return to caller
    if (ret && clearOSF) {              //clear OSF if it's set and the caller wants to clear it
        writeRTC( RTC_STATUS, s & ~_BV(OSF) );
    }
    return ret;
}

/*----------------------------------------------------------------------*
 * Returns the temperature in Celsius times four.                       *
 *----------------------------------------------------------------------*/
int DS3232RTC::temperature(void)
{
    union int16_byte {
        int i;
        byte b[2];
    } rtcTemp;
    
    rtcTemp.b[0] = readRTC(TEMP_LSB);
    rtcTemp.b[1] = readRTC(TEMP_MSB);
    return rtcTemp.i >> 6;
}

/*----------------------------------------------------------------------*
 * Decimal-to-BCD conversion                                            *
 *----------------------------------------------------------------------*/
uint8_t DS3232RTC::dec2bcd(uint8_t n)
{
    return n + 6 * (n / 10);
}

/*----------------------------------------------------------------------*
 * BCD-to-Decimal conversion                                            *
 *----------------------------------------------------------------------*/
uint8_t __attribute__ ((noinline)) DS3232RTC::bcd2dec(uint8_t n)
{
    return n - 6 * (n >> 4);
}

DS3232RTC DSRTC = DS3232RTC();            //instantiate an RTC object
