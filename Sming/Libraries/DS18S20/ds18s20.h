/*
 * ds18s20.h
 *
 *  Created on: 01-09-2015
 *      Author: flexiti and Anakod
 */

#ifndef READFROMDS_H_
#define READFROMDS_H_

/** @defgroup   DS18S20 DS18S20 Temperature Sensor
 *  @brief  This library provides access to DS18S20 temperature sensors connected via 1-Wire bus to a single GPIO
 *          The DS18S20 can run in several modes, with varying degrees of resolution. The highest resolution is 12-bit which provides 0.0625C resolution.
            12-bit measurement takes 750ms. With 4 sensors connected, measurement will take 3s.
 *  @ingroup    libraries
 *  @{
*/
#include <Libraries/OneWire/OneWire.h>

#define MAX_SENSORS 4         ///< Maximum quantity of sensors to read

// OneWire commands

#define COPYSCRATCH     0x48  // Copy EEPROM
#define READSCRATCH     0xBE  // Read EEPROM
#define WRITESCRATCH    0x4E  // Write EEPROM
#define RECALLSCRATCH   0xB8  // Reload last
#define READPOWERSUPPLY 0xB4  // parasite power
#define ALARMSEARCH     0xEC  // Query for alarm
#define STARTCONVO      0x44  // temperature reading

#define DS1820_WORK_PIN 2	// default DS1820 on GPIO2, can be changed by Init

/** @brief  Definition of callback function called on completion of measurement of all DS18S20 sensors
    @note   Example: void onMeasurment() { ... };
*/
typedef Delegate<void()> DS18S20CompletedDelegate;

/** @brief  This class implements access to the DS18x20 range of temperature sensors
*/
class DS18S20
{
public:
    /** @brief  Instantiate a DS18S20 object
    */
    DS18S20(uint8_t workPin = DS1820_WORK_PIN);

    /** @brief  Initiate communication on 1-wire bus
    *   @param  GPIO pin acting as 1-wire bus
    */
	void Init(uint8_t);

    /** @brief  Start measurement of all connected sensors
    *   @note   Scans for all connected sensors, measures value from each then calls the registered callback function
    */
	void StartMeasure();

    /** @brief  Register the callback function that is run when measurement is complete
    *   @param  Name of the callback function
    *   @note   Callback function must be a DS18S20CommpleteDelegate type
    */
	void RegisterEndCallback(DS18S20CompletedDelegate);

	/** @brief  Unregister the callback function to avoid activity after measurement is complete
	*/
	void UnRegisterCallback();             			//Unset conversion end function

    /** @brief  Get the value of the last measurment from a sensor
    *   @param  Index of sensor to retrieve value from
    *   @return Temperature value in degrees Celsius or zero for invalid sensor index
    *   @note   Call IsValidTemperature() to check value is valid before calling this function
    */
	float GetCelsius(uint8_t);

    /** @brief  Get the value of the last measurment from a sensor
    *   @param  Index of sensor to retrieve value from
    *   @return Temperature value in degrees Fahrenheit or zero for invalid sensor index
    *   @note   Call IsValidTemperature() to check value is valid before calling this function
    */
	float GetFahrenheit(uint8_t);

	/** @brief  Check if the last measurement for a sensor is valid
	*   @param  Index of sensor to check
	*   @return True if last measured temperature is valid, false for invalid value or invalid sensor index
	*   @note   Call this function before reading value to ensure the value is valid
	*   @todo   Initial code review suggests this is set once but never rechecked so may not be accurate
	*/
	bool IsValidTemperature(uint8_t);

    /** @brief  Check if measurement is in progress
    *   @return True if measurement is in progress
    */
	bool MeasureStatus();

    /** @brief  Get the ID (1-wire address) of a sensor
    *   @param  Index of sensor
    *   @return ID of sensor
    */
	uint64_t GetSensorID(uint8_t);

    /** @brief  Get the quantity of sensors detected during last measurement
    *   @return Quantity of detected sensors
    *   @note   Maximum quantity of sensors is defined in library header file  by MAX_SENSORS
    */
	uint8_t GetSensorsCount();				//how many the sensors detected

	virtual ~DS18S20();

private:

	void DoMeasure();
	void DoSearch();
	void StartReadNext();
	uint8_t FindAlladdresses();

private:
	bool InProgress = false;
	bool ValidTemperature[MAX_SENSORS];
	uint8_t addr[8];
	uint8_t type_s[MAX_SENSORS];
	uint8_t data[12];
	uint64_t addresses[MAX_SENSORS];
	uint8_t numberOf=0;
	uint8_t numberOfread=0;

	DS18S20CompletedDelegate readEndCallback;

	Timer DelaysTimer;


	float celsius[MAX_SENSORS], fahrenheit[MAX_SENSORS];

	OneWire* ds = nullptr;
};

/** @} */

#endif /* READFROMDS_H_ */

