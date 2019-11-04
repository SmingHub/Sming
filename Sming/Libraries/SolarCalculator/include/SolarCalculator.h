/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SolarCalculator.h - Calculation of apparent time of sunrise and sunset
 *
 * Note: Months are 1-based
 *
 ****/

#pragma once

#include <DateTime.h>

/**
 * @brief A location is required to compute solar times
 */
struct SolarRef {
	float latitude;
	float longitude;
};

class SolarCalculator
{
public:
	/**
     * @brief Default constructor, uses Royal Observatory, Greenwich as default
     */
	SolarCalculator()
	{
	}

	/**
     * @brief Perform calculations using the given solar reference
     */
	SolarCalculator(const SolarRef& ref) : ref(ref)
	{
	}

	/**
	 * @brief Get the current location reference in use
	 */
	const SolarRef& getRef() const
	{
		return ref;
	}

	/**
	 * @brief Set the location reference for calculations
	 */
	void setRef(const SolarRef& ref)
	{
		this->ref = ref;
	}

	/**
     * @briefCalculate a sunrise or sunset figure for a given day.
     * @param isRise true for sunrise, false for sunset
     * @param y Absolute year
     * @param m Month number (1 - 12)
     * @param d Day of month (1 - 31)
     * @retval int Minutes since midnight, -1 if there is no sunrise/sunset
     * @{
     */
	int sunRiseSet(bool isRise, int y, int m, int d);

	int sunrise(int y, int m, int d)
	{
		return sunRiseSet(true, y, m, d);
	}

	int sunset(int y, int m, int d)
	{
		return sunRiseSet(false, y, m, d);
	}

	/**
     * @}
     */

private:
	/*
	 * Though most time zones are offset by whole hours, there are a few zones
	 * offset by 30 or 45 minutes, so the argument must be declared as a float.
	 *
	 * Royal Observatory, Greenwich, seems an appropriate default setting
	 */
	SolarRef ref = {51.4769, 0.0005};
};
