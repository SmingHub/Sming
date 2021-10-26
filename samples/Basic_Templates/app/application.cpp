#include <SmingCore.h>
#include <Data/Stream/SectionTemplate.h>
#include <FlashString/Stream.hpp>
#include <Data/CsvReader.h>
#include "CsvTemplate.h"

namespace
{
/*
 * Templates are small and benefit from faster access as they're read repeatedly,
 * so they're stored in program memory.
 */
namespace Resource
{
IMPORT_FSTR(cars_txt, PROJECT_DIR "/templates/cars.txt")
IMPORT_FSTR(classics_json, PROJECT_DIR "/templates/classics.json")
IMPORT_FSTR(classics_xml, PROJECT_DIR "/templates/classics.xml")
} // namespace Resource

/*
 * CSV files are large and read strictly sequentially so store these in filesystem
 */
namespace Filename
{
DEFINE_FSTR(cars_csv, "cars.csv")
DEFINE_FSTR(classics_csv, "classics.csv")
} // namespace Filename

/*
 * Demonstrate use of custom Template class using CSV source data.
 * Records are filtered by the template.
 */
void printCars()
{
	// Set up our templating class
	CsvTemplate tmpl(new FSTR::Stream(Resource::cars_txt), // The template source
					 new FileStream(Filename::cars_csv)	// The CSV data source
	);

	// Set the variable used by the template to filter records
	tmpl.setVar("make_filter", "Volkswagen");

	// Dump result to terminal
	Serial.copyFrom(&tmpl);
}

/*
 * Demonstrate using callbacks instead of CsvTemplate class,
 * and performing record filtering outside of template.
 */
void printClassics(const FlashString& templateSource, Format::Formatter& formatter)
{
	// The CSV data source
	CsvReader csv(new FileStream(Filename::classics_csv));

	// Use a regular SectionTemplate class to process the template
	SectionTemplate tmpl(new FSTR::Stream(templateSource));

	// We're going to filter the data based on publication year
	String year_filter("1996");
	// Provide this to the template so it can show it in the header
	tmpl.setVar(_F("year_filter"), year_filter);
	// Improve speed by pre-fetching the filter column index
	unsigned yearColumn = csv.getColumn(_F("bibliography.publication.year"));

	// Set up callback to handle fetching/filtering records
	tmpl.onNextRecord([&]() -> bool {
		if(tmpl.sectionIndex() == 1) {
			while(csv.next()) {
				// Could also use a more generic (but slower) approach:
				//   if(year_filter == tmpl.getValue("bibliography.publication.year"))

				if(year_filter == csv.getValue(yearColumn)) {
					return true;
				}
			}
			return false;
		}
		return tmpl.recordIndex() < 0;
	});

	// Set up callback to fetch values from CSV record
	tmpl.onGetValue([&](const char* name) -> String {
		String s = csv.getValue(name);
		formatter.escape(s);
		return s;
	});

	// Dump result to terminal
	Serial.copyFrom(&tmpl);
}

} // namespace

void init()
{
#if DEBUG_BUILD
	Serial.begin(COM_SPEED_SERIAL);

	Serial.systemDebugOutput(true);
	debug_i("\n\n********************************************************\n"
			"Hello\n");
#endif

	fwfs_mount();

	printCars();
	printClassics(Resource::classics_json, Format::json);
	printClassics(Resource::classics_xml, Format::xml);
}
