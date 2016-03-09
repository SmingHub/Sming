/*----------------------------------------------------------------------*
 * DS3232RTC.h - Arduino library for the Maxim Integrated DS3232        *
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

#ifndef DS3232RTC_h
#define DS3232RTC_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h> 
#else
#include <WProgram.h> 
#endif


//HACK
typedef enum {
    tmSecond, tmMinute, tmHour, tmWday, tmDay,tmMonth, tmYear, tmNbrFields
} tmByteFields;

typedef struct  { 
  uint8_t Second; 
  uint8_t Minute; 
  uint8_t Hour; 
  uint8_t Wday;   // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month; 
  uint8_t Year;   // offset from 1970; 
} 	tmElements_t, TimeElements, *tmElementsPtr_t;

//convenience macros to convert to and from tm years 
#define  tmYearToCalendar(Y) ((Y) + 1970)  // full four digit year 
#define  CalendarYrToTm(Y)   ((Y) - 1970)
#define  tmYearToY2k(Y)      ((Y) - 30)    // offset is from 2000
#define  y2kYearToTm(Y)      ((Y) + 30)

/* low level functions to convert to and from system time                     */
void breakTime(time_t time, tmElements_t &tm);  // break time_t into elements
time_t makeTime(tmElements_t &tm);  // convert time elements into time_t

//#include <DateTime.h>

//DS3232 I2C Address
#define RTC_ADDR 0x68

//DS3232 Register Addresses
#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x01
#define RTC_HOURS 0x02
#define RTC_DAY 0x03
#define RTC_DATE 0x04
#define RTC_MONTH 0x05
#define RTC_YEAR 0x06
#define ALM1_SECONDS 0x07
#define ALM1_MINUTES 0x08
#define ALM1_HOURS 0x09
#define ALM1_DAYDATE 0x0A
#define ALM2_MINUTES 0x0B
#define ALM2_HOURS 0x0C
#define ALM2_DAYDATE 0x0D
#define RTC_CONTROL 0x0E
#define RTC_STATUS 0x0F
#define RTC_AGING 0x10
#define TEMP_MSB 0x11
#define TEMP_LSB 0x12
#define SRAM_START_ADDR 0x14    //first SRAM address
#define SRAM_SIZE 236           //number of bytes of SRAM

//Alarm mask bits
#define A1M1 7
#define A1M2 7
#define A1M3 7
#define A1M4 7
#define A2M2 7
#define A2M3 7
#define A2M4 7

//Control register bits
#define EOSC 7
#define BBSQW 6
#define CONV 5
#define RS2 4
#define RS1 3
#define INTCN 2
#define A2IE 1
#define A1IE 0

//Status register bits
#define OSF 7
#define BB32KHZ 6
#define CRATE1 5
#define CRATE0 4
#define EN32KHZ 3
#define BSY 2
#define A2F 1
#define A1F 0

//Square-wave output frequency (TS2, RS1 bits)
enum SQWAVE_FREQS_t {SQWAVE_1_HZ, SQWAVE_1024_HZ, SQWAVE_4096_HZ, SQWAVE_8192_HZ, SQWAVE_NONE};

//Alarm masks
enum ALARM_TYPES_t {
    ALM1_EVERY_SECOND = 0x0F,
    ALM1_MATCH_SECONDS = 0x0E,
    ALM1_MATCH_MINUTES = 0x0C,     //match minutes *and* seconds
    ALM1_MATCH_HOURS = 0x08,       //match hours *and* minutes, seconds
    ALM1_MATCH_DATE = 0x00,        //match date *and* hours, minutes, seconds
    ALM1_MATCH_DAY = 0x10,         //match day *and* hours, minutes, seconds
    ALM2_EVERY_MINUTE = 0x8E,
    ALM2_MATCH_MINUTES = 0x8C,     //match minutes
    ALM2_MATCH_HOURS = 0x88,       //match hours *and* minutes
    ALM2_MATCH_DATE = 0x80,        //match date *and* hours, minutes
    ALM2_MATCH_DAY = 0x90,         //match day *and* hours, minutes
};

#define ALARM_1 1                  //constants for calling functions
#define ALARM_2 2

//Other
#define DS1307_CH 7                //for DS1307 compatibility, Clock Halt bit in Seconds register
#define HR1224 6                   //Hours register 12 or 24 hour mode (24 hour mode==0)
#define CENTURY 7                  //Century bit in Month register
#define DYDT 6                     //Day/Date flag bit in alarm Day/Date registers

class DS3232RTC
{
    public:
        DS3232RTC();
        static time_t get(void);    //must be static to work with setSyncProvider() in the Time library
        byte set(time_t t);
        static byte read(tmElements_t &tm);
        byte write(tmElements_t &tm);
        byte writeRTC(byte addr, byte *values, byte nBytes);
        byte writeRTC(byte addr, byte value);
        byte readRTC(byte addr, byte *values, byte nBytes);
        byte readRTC(byte addr);
        void setAlarm(ALARM_TYPES_t alarmType, byte seconds, byte minutes, byte hours, byte daydate);
        void setAlarm(ALARM_TYPES_t alarmType, byte minutes, byte hours, byte daydate);
        void alarmInterrupt(byte alarmNumber, bool alarmEnabled);
        bool alarm(byte alarmNumber);
        void squareWave(SQWAVE_FREQS_t freq);
        bool oscStopped(bool clearOSF = true);  //defaults to clear the OSF bit if argument not supplied
        int temperature(void);

    private:
        uint8_t dec2bcd(uint8_t n);
        static uint8_t bcd2dec(uint8_t n);
};

extern DS3232RTC DSRTC;

#endif
