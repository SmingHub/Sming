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

	bool nextRecord() override
	{
		if(sectionIndex() == 1) {
			return csv.next();
		}

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
