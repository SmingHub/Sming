#ifndef LOCALE_H_INCLUDED
#define LOCALE_H_INCLUDED

//Define unique values for each locale (try to use ISD codes if appropriate)
#define LOCALE_EN_GB 44
#define LOCALE_EN_AU 61
#define LOCALE_EN_US 1
#define LOCALE_DE_DE 49
#define LOCALE_FR_FR 33

//Default values (en-gb)
#define LOCALE_MONTH_NAMES                                                                                             \
	"January\0February\0March\0April\0May\0June\0July\0August\0September\0October\0November\0December"
#define LOCALE_DAY_NAMES "Sunday\0Monday\0Tuesday\0Wednesday\0Thursday\0Friday\0Saturday"
#define LOCALE_DATE "%d/%m/%Y"
#define LOCALE_TIME "%H:%M:%S"
#define LOCALE_DATE_TIME "%a %b %d %H:%M:%S %Y"

#ifndef LOCALE
#define LOCALE LOCALE_EN_GB
#endif // LOCALE

//Austailia
#ifdef LOCALE LOCALE_EN_AU
#endif // LOCALE_EN_AU

//USA
#ifdef LOCALE LOCALE_EN_US
#define LOCALE_DATE "%m/%d/%Y"
#endif // LOCALE_EN_US

//Germany
#ifdef LOCALE LOCALE_DE_DE
#define LOCALE_MONTH_NAMES                                                                                             \
	"Januar\0Februar\0März\0April\0Mai\0Juni\0Juli\0August\0September\0Oktober\0November\0Dezember"
#define LOCALE_DAY_NAMES "Sonntag\0Montag\0Dienstag\0Mittwoch\0Donnerstag\0Freitag\0Samstag"
#define LOCALE_DATE "%d.%m.%Y"
#endif // LOCALE_DE_DE

//France
#ifdef LOCALE LOCALE_FR_FR
#define LOCALE_MONTH_NAMES                                                                                             \
	"janvier\0février\0mars\0avril\0mai\0juin\0juillet\0août\0septembre\0octobre\0novembre\0décembre"
#define LOCALE_DAY_NAMES "dimanche\0lundi\0mardi\0mercredi\0jeudi\0vendredi\0samedi"
#define LOCALE_DATE "%d-%m-%Y"
#endif // LOCALE_FR_FR

#endif // LOCALE_H_INCLUDED
