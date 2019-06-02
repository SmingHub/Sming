/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SmingLocale.h
 *
 * Localization is defined within SmingLocale.h
 *	Each locale has a unique ID (usually its international dial code, e.g. GB=44
 *	The default locale is GB and the default values are those used by GB.
 *	To add a new locale:
 *		#define LOCALE_xx_yy zz (where xx_yy is the locale identifier and zz is the IDC)
 *		Override any variation from GB settings within a "#elifdef LOCALE_xx_yy zz" block
 *	Default settings are at end of file
 */

#pragma once

// Define unique values for each locale (try to use ISD codes if appropriate)
#define LOCALE_EN_US 1
#define LOCALE_FR_FR 33
#define LOCALE_EN_GB 44
#define LOCALE_DE_DE 49
#define LOCALE_EN_AU 61

#ifndef LOCALE
#define LOCALE LOCALE_EN_GB
#endif // LOCALE

#if LOCALE == LOCALE_EN_US

#define LOCALE_DATE "%m/%d/%Y"
#define LOCALE_DATE_TIME "%a %b %d %H:%M:%S %Y"

#elif LOCALE == LOCALE_FR_FR

#define LOCALE_MONTH_NAMES                                                                                             \
	"janvier\0f�vrier\0mars\0avril\0mai\0juin\0juillet\0ao�t\0septembre\0octobre\0novembre\0d�cembre"
#define LOCALE_DAY_NAMES "dimanche\0lundi\0mardi\0mercredi\0jeudi\0vendredi\0samedi"

#elif LOCALE == LOCALE_DE_DE

#define LOCALE_MONTH_NAMES                                                                                             \
	"Januar\0Februar\0M�rz\0April\0Mai\0Juni\0Juli\0August\0September\0Oktober\0November\0Dezember"
#define LOCALE_DAY_NAMES "Sonntag\0Montag\0Dienstag\0Mittwoch\0Donnerstag\0Freitag\0Samstag"
#define LOCALE_DATE "%d.%m.%Y"

#elif LOCALE == LOCALE_EN_AU

// Austrailia is same as GB

#endif // LOCALE

// Defaults (GB)
#ifndef LOCALE_MONTH_NAMES
// String array with full month names, starting with January, separated by '\0'
#define LOCALE_MONTH_NAMES                                                                                             \
	"January\0February\0March\0April\0May\0June\0July\0August\0September\0October\0November\0December"
#endif // LOCALE_MONTH_NAMES

#ifndef LOCALE_DAY_NAMES
// String array with full day names, starting with Sunday, separated by '\0'
#define LOCALE_DAY_NAMES "Sunday\0Monday\0Tuesday\0Wednesday\0Thursday\0Friday\0Saturday"
#endif // LOCALE_DAY_NAMES

#ifndef LOCALE_DATE
// String with short date format (see DateTime::format for options - this implements %x format)
#define LOCALE_DATE "%d/%m/%Y"
#endif // LOCALE_DATE

#ifndef LOCALE_TIME
// String with time format (see DateTime::format for options - this implements %X format)
#define LOCALE_TIME "%H:%M:%S"
#endif // LOCALE_TIME

#ifndef LOCALE_DATE_TIME
// String with time and date format (see DateTime::format for options - this implements %c format)
#define LOCALE_DATE_TIME "%a %d %b %Y %X"
#endif // LOCALE_DATE_TIME
