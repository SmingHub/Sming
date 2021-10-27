#pragma once

#include <Data/CsvReader.h>
#include <Data/Stream/SectionTemplate.h>

/**
 * @brief SectionTemplate implementation for data stored in CSV file
 */
class CsvTemplate : public SectionTemplate
{
public:
	CsvTemplate(IDataSourceStream* templateSource, IDataSourceStream* csvSource)
		: SectionTemplate(templateSource), csv(csvSource)
	{
	}

	// Return true if we have a new valid record, false if not
	bool nextRecord() override
	{
		// Content section we fetch the next data record, if there is one
		if(sectionIndex() == 1) {
			return csv.next();
		}

		// This code emits the header and footer sections exactly once
		// `recordIndex` starts at -1 (before first record)
		return recordIndex() < 0;
	}

protected:
	String getValue(const char* name) override
	{
		String s = SectionTemplate::getValue(name);
		return s ?: csv.getValue(name);
	}

private:
	CsvReader csv;
};
