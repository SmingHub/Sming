/*
 * Name: libDHT
 * License: MIT license. See details in DHT.cpp.
 * Location: https://github.com/ADiea/libDHT
 * Maintainer: ADiea (https://github.com/ADiea)
 *
 * Descr: Arduino compatible DHT 11/22, AM2302 lib with dewpoint,
 *        heat-index and other goodies.
 *
 * Features & History:
 * See DHT.cpp
 */
#ifndef DHT_H
#define DHT_H

#if ARDUINO >= 100
 #include "Arduino.h"

 #define PULLUP_PIN(x) pinMode(x, INPUT_PULLUP)

#else
 #include "WProgram.h"

 #define PULLUP_PIN(x) pinMode(x, INPUT); \
					   digitalWrite(x, HIGH)
#endif

#define DHT_CELSIUS 0
#define DHT_FARENHEIT 1
#define DHT_RUNTIME 2

/*************** USER DEFINED SWITCHES ***************/

//Change to 1 for debug output
#define DHT_DEBUG 0

//If set to 1, will not re trigger a read if attempting to use old readings
#define NO_AUTOREFRESH 0

/* Your choices are:
 * DHT_CELSIUS: Smallest code size.
 * DHT_FARENHEIT: Bigger code size. (+84 bytes on ESP8266 arch)
 * DHT_RUNTIME: Most flexible, compatible with Adafruit's lib.
 * 				(+124 bytes over DHT_CELSIUS for ESP8266 arch)
 * */
#define DHT_TEMPERATURE DHT_RUNTIME

/*************** SYSTEM CONSTANTS ***************/

/*From datasheet: http://www.micro4you.com/files/sensor/DHT11.pdf
 * '0' if HIGH lasts 26-28us,
 * '1' if HIGH lasts 70us */
#define ONE_DURATION_THRESH_US 30

#define DHTLIB_DHT11_WAKEUP 18
#define DHTLIB_DHT22_WAKEUP 5

// how many timing transitions we need
// to keep track of. 2 * 40 bits + extra
#define MAXTIMINGS 85

#define DHT_AUTO 0
#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

#define READ_INTERVAL_DHT11_DSHEET 1000
#define READ_INTERVAL_DHT22_DSHEET 2000
#define READ_INTERVAL_DONT_CARE 2200 /*safe value*/
#define READ_INTERVAL_LONG 4000

#define DEW_ACCURATE 0
#define DEW_FAST 1
#define DEW_ACCURATE_FAST 2
#define DEW_FASTEST 3

#define WAKEUP_DHT11 18
#define WAKEUP_DHT22 1

#define LAST_VALUE -1

// Reference: http://epb.apogee.net/res/refcomf.asp
enum ComfortState
{
	Comfort_OK = 0,
	Comfort_TooHot = 1,
	Comfort_TooCold = 2,
	Comfort_TooDry = 4,
	Comfort_TooHumid = 8,
	Comfort_HotAndHumid = 9,
	Comfort_HotAndDry = 5,
	Comfort_ColdAndHumid = 10,
	Comfort_ColdAndDry = 6
};

enum ErrorDHT
{
	errDHT_OK = 0,
	errDHT_Timeout,
	errDHT_Checksum,
	errDHT_Other,
};

struct TempAndHumidity
{
	float temp;
	float humid;
};

struct ComfortProfile
{
	//Represent the 4 line equations:
	//dry, humid, hot, cold, using the y = mx + b formula
	float m_tooHot_m, m_tooHot_b;
	float m_tooCold_m, m_tooHCold_b;
	float m_tooDry_m, m_tooDry_b;
	float m_tooHumid_m, m_tooHumid_b;

	inline bool isTooHot(float temp, float humidity)
		{return (temp > (humidity * m_tooHot_m + m_tooHot_b));}
	inline bool isTooHumid(float temp, float humidity)
		{return (temp > (humidity * m_tooHumid_m + m_tooHumid_b));}
	inline bool isTooCold(float temp, float humidity)
		{return (temp < (humidity * m_tooCold_m + m_tooHCold_b));}
	inline bool isTooDry(float temp, float humidity)
		{return (temp < (humidity * m_tooDry_m + m_tooDry_b));}

	inline float distanceTooHot(float temp, float humidity)
		{return temp - (humidity * m_tooHot_m + m_tooHot_b);}
	inline float distanceTooHumid(float temp, float humidity)
		{return temp - (humidity * m_tooHumid_m + m_tooHumid_b);}
	inline float distanceTooCold(float temp, float humidity)
		{return (humidity * m_tooCold_m + m_tooHCold_b) - temp;}
	inline float distanceTooDry(float temp, float humidity)
		{return (humidity * m_tooDry_m + m_tooDry_b) - temp;}
};

class DHT
{
public:
	/*********************** STATIC METHODS ***********************/
	/*can be used without an object ex DHT::convertCtoF(33.2)     */

	static inline float convertCtoF(float c){ return c * 1.8f + 32; }
	static inline float convertFtoC(float f){ return (f-32)/1.8f; }

	/*********************** REGULAR METHODS ***********************/
	/*must be called with an object ex dht.begin()                 */

	/**
	 * Constructor.
	 * @param pin - the GPIO [in number the sensor is hooked up to
	 * @param type - the sensor type
	 * @param minIntervalRead - The minimum time between reads in ms
	 * */
	DHT(uint8_t pin,
		uint8_t type = DHT_AUTO,
		uint16_t minIntervalRead = READ_INTERVAL_DONT_CARE)
		: m_kSensorPin(pin), m_kSensorType(type), m_minIntervalRead(minIntervalRead)
	{
		m_lastError = errDHT_Other;
		m_lastTemp = NAN;
		m_lastHumid = NAN;

		if (DHT11 == m_kSensorType)
		{
			if (READ_INTERVAL_DONT_CARE == minIntervalRead)
			{
				m_minIntervalRead = READ_INTERVAL_DHT11_DSHEET;
			}
			m_wakeupTimeMs = WAKEUP_DHT11;
		}
		else if (DHT22 == m_kSensorType || DHT21 == m_kSensorType)
		{
			if (READ_INTERVAL_DONT_CARE == minIntervalRead)
			{
				m_minIntervalRead = READ_INTERVAL_DHT22_DSHEET;
			}
			m_wakeupTimeMs = WAKEUP_DHT22;
		}

		//Set default comfort profile.

		//In computing these constants the following reference was used
		//http://epb.apogee.net/res/refcomf.asp
		//It was simplified as 4 straight lines and added very little skew on
		//the vertical lines (+0.1 on x for C,D)
		//The for points used are(from top left, clock wise)
		//A(30%, 30*C) B(70%, 26.2*C) C(70.1%, 20.55*C) D(30.1%, 22.22*C)
		//On the X axis we have the rel humidity in % and on the Y axis the temperature in *C

		//Too hot line AB
		m_comfort.m_tooHot_m = -0.095;
		m_comfort.m_tooHot_b = 32.85;
		//Too humid line BC
		m_comfort.m_tooHumid_m =  -56.5;
		m_comfort.m_tooHumid_b = 3981.2;
		//Too cold line DC
		m_comfort.m_tooCold_m = -0.04175;
		m_comfort.m_tooHCold_b = 23.476675;
		//Too dry line AD
		m_comfort.m_tooDry_m = -77.8;
		m_comfort.m_tooDry_b = 2364;
	};

	/**
	 * Must be called once at startup
	 * */
	void begin();

	/**
	 * Read temperature. Compatible with Adafruit's lib(only for DHT_RUNTIME)
	 * @param bFarenheit - true if a conversion to Farenheit is desired
	 * */
	float readTemperature(
#if DHT_TEMPERATURE == 	DHT_RUNTIME
			bool bFarenheit = false
#endif
	);

	/**
	 * Read humidity. Compatible with Adafruit's lib
	 * */
	float readHumidity();

	/**
	 * Read both temperature and humidity.
	 * @param destReading - will hold the temp and humidity readings
	 * @param bFarenheit - true if a conversion to Farenheit is desired
	 * */
	bool readTempAndHumidity(TempAndHumidity& destReading
#if DHT_TEMPERATURE == 	DHT_RUNTIME
			, bool bFarenheit = false
#endif
	);

	//Get and set the current comfort profile
	ComfortProfile getComfortProfile() {return m_comfort;}
	void setComfortProfile(ComfortProfile& c) {m_comfort = c;}

	/* Interrogate the current comfort profile for cold,hot,humid,dry states
	*  If default LAST_VALUE value is used, will take into account the last read values.
	*
	*  Automatically starts a new read if values are old.
	*  This can be disabled by the global switch NO_AUTOREFRESH
	*  */
	inline bool isTooHot(float temp = LAST_VALUE, float humidity = LAST_VALUE)
		{return m_comfort.isTooHot(temp, humidity);}
	inline bool isTooHumid(float temp = LAST_VALUE, float humidity = LAST_VALUE)
		{return m_comfort.isTooHumid(temp, humidity);}
	inline bool isTooCold(float temp = LAST_VALUE, float humidity = LAST_VALUE)
		{return m_comfort.isTooCold(temp, humidity);}
	inline bool isTooDry(float temp = LAST_VALUE, float humidity = LAST_VALUE)
		{return m_comfort.isTooDry(temp, humidity);}

	/**
	 * Get the calculated HEAT INDEX
	 * @param tempCelsius - temp in *C. Default uses the last temp reading.
	 * 						If the reading is old, a read() is triggered
	 * 						This can be disabled with the NO_AUTOREFRESH switch
	 * @param percentHumidity - humidity 0..100. Default uses the last humid reading.
	 * 						If the reading is old, a read() is triggered
	 * 						This can be disabled with the NO_AUTOREFRESH switch
	 * @param bFarenheit - true if a conversion to Farenheit is desired
	 */
	float getHeatIndex(float tempCelsius = LAST_VALUE, float percentHumidity = LAST_VALUE
#if DHT_TEMPERATURE == 	DHT_RUNTIME
					, bool bFarenheit = false
#endif
	);

	/**
	 * Get the calculated DEW POINT
	 * @param tempCelsius - temp in *C. Default uses the last temp reading.
	 * 						If the reading is old, a read() is triggered
	 * 						This can be disabled with the NO_AUTOREFRESH switch
	 * @param percentHumidity - humidity 0..100. Default uses the last humid reading.
	 * 						If the reading is old, a read() is triggered
	 * 						This can be disabled with the NO_AUTOREFRESH switch
	 * @param algType - Algorithm type to use. See DHT.c for details
	 */
	double getDewPoint(uint8_t algType = DEW_ACCURATE_FAST,
						float tempCelsius = LAST_VALUE,
						float percentHumidity = LAST_VALUE
#if DHT_TEMPERATURE == 	DHT_RUNTIME
						, bool bFarenheit = false
#endif
	);

	/**
	 * Get the heuristic COMFORT RATIO (0=unconfortable..100=confortable)
	 * 	based on current comfort profile.
	 * @param tempCelsius - temp in *C. Default uses the last temp reading.
	 * 						If the reading is old, a read() is triggered
	 * 						This can be disabled with the NO_AUTOREFRESH switch
	 * @param percentHumidity - humidity 0..100. Default uses the last humid reading.
	 * 						If the reading is old, a read() is triggered
	 * 						This can be disabled with the NO_AUTOREFRESH switch
	 * @param destComfStatus - will receive a comfort classification
	 */
	float getComfortRatio(ComfortState& destComfStatus,
						 float temp = LAST_VALUE,
						 float percentHumidity = LAST_VALUE);

	/**
	 * Gets the last occurred error.
	 */
	inline ErrorDHT getLastError() { return m_lastError; }

private:
	bool read();
	void updateInternalCache();
	
	uint8_t m_kSensorPin, m_kSensorType;
	uint8_t m_data[6];
	unsigned long m_lastreadtime;
	uint8_t m_wakeupTimeMs;

	ComfortProfile m_comfort;

	ErrorDHT m_lastError;

	//The datasheet advises to read no more than one every 2 seconds.
	//However if reads are done at greater intervals the sensor's output
	//will be less subject to self-heating
	//Reference: http://www.kandrsmith.org/RJS/Misc/dht_sht_how_fast.html
	uint16_t m_minIntervalRead;

	//internal cache, last read values
	float m_lastTemp, m_lastHumid;
};
#endif
