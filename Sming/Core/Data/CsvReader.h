/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CsvReader.h
 *
 * @author: 2021 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "Stream/DataSourceStream.h"
#include "CStringArray.h"
#include <memory>

/**
 * @brief Class to parse a CSV file
 *
 * Spec: https://www.ietf.org/rfc/rfc4180.txt
 *
 * 1. Each record is located on a separate line
 * 2. Line ending for last record in the file is optional
 * 3. Field headings are provided either in the source data or in constructor (but not both)
 * 4. Fields separated with ',' and whitespace considered part of field content
 * 5. Fields may or may not be quoted - if present, will be removed during parsing
 * 6. Fields may contain line breaks, quotes or commas
 * 7. Quotes may be escaped thus "" if field itself is quoted
 *
 * Additional features:
 *
 * - Line breaks can be \n or \r\n
 * - Escapes codes within fields will be converted: \n \r \t \", \\
 * - Field separator can be changed in constructor
 */
class CsvReader
{
public:
	/**
	 * @brief Construct a CSV reader
	 * @param source Stream to read CSV text from
	 * @param fieldSeparator
	 * @param headings Required if source data does not contain field headings as first row
	 * @param maxLineLength Limit size of buffer to guard against malformed data
	 */
	CsvReader(IDataSourceStream* source, char fieldSeparator = ',', const CStringArray& headings = nullptr,
			  size_t maxLineLength = 2048)
		: fieldSeparator(fieldSeparator), userHeadingsProvided(headings), maxLineLength(maxLineLength),
		  headings(headings)
	{
		this->source.reset(source);
		reset();
	}

	/**
	 * @brief Reset reader to start of CSV file
	 *
	 * Cursor is set to 'before start'.
	 * Call 'next()' to fetch first record.
	 */
	void reset();

	/**
	 * @brief Seek to next record
	 */
	bool next()
	{
		return readRow();
	}

	/**
	 * @brief Get number of columns
	 */
	unsigned count() const
	{
		return headings.count();
	}

	/**
	 * @brief Get a value from the current row
	 * @param index Column index, starts at 0
	 * @retval const char* nullptr if index is not valid
	 */
	const char* getValue(unsigned index)
	{
		return row[index];
	}

	/**
	 * @brief Get a value from the current row
	 * @param index Column name
	 * @retval const char* nullptr if name is not found
	 */
	const char* getValue(const char* name)
	{
		return getValue(getColumn(name));
	}

	/**
	 * @brief Get index of columnn given its name
	 * @param name Column name to find
	 * @retval int -1 if name is not found
	 */
	int getColumn(const char* name)
	{
		return headings.indexOf(name);
	}

	/**
	 * @brief Determine if row is valid
	 */
	explicit operator bool() const
	{
		return bool(row);
	}

	/**
	 * @brief Get headings
	 */
	const CStringArray& getHeadings() const
	{
		return headings;
	}

	/**
	 * @brief Get current row
	 */
	const CStringArray& getRow() const
	{
		return row;
	}

private:
	bool readRow();

	std::unique_ptr<IDataSourceStream> source;
	char fieldSeparator;
	bool userHeadingsProvided;
	size_t maxLineLength;
	CStringArray headings;
	CStringArray row;
};
