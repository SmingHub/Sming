/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SectionStream.h
 *
 * @author mikee47 <mike@sillyhouse.net> Nov 2020
 *
 ****/

#pragma once

#include "DataSourceStream.h"
#include <memory>

/**
 * @brief Presents each section within a source stream as a separate stream
 *
 * Sections are (by default) marked {!SECTION} ... {/SECTION}
 * This is typically used with templating but can be used with any stream type provided
 * the tags do not conflict with content.
 */
class SectionStream : public IDataSourceStream
{
public:
	struct Section {
		uint32_t start; // Within stream
		uint32_t size;
		uint32_t recordCount;
		int recordIndex;

		uint32_t end() const
		{
			return start + size;
		}
	};

	/**
	 * @brief Application notification callback when section changes
	 */
	using NextSection = Delegate<void()>;

	/**
	 * @brief Application callback to move to next record
	 * @retval bool true to emit section, false to skip
	 */
	using NextRecord = Delegate<bool()>;

	/**
	 * @brief Construct a section stream with default options
	 */
	SectionStream(IDataSourceStream* source, uint8_t maxSections = 5)
		: SectionStream(source, maxSections, F("{SECTION}"), F("{/SECTION}"))
	{
	}

	/**
	 * @brief Construct a section stream
	 * @param source Contains all section data, must support random seeking
	 * @param startTag Unique text used to mark start of a section
	 * @param endTag Marks end of a section
	 */
	SectionStream(IDataSourceStream* source, uint8_t maxSections, const String& startTag, const String& endTag)
		: stream(source), startTag(startTag), endTag(endTag)
	{
		scanSource(maxSections);
	}

	~SectionStream()
	{
		delete stream;
	}

	int available() override
	{
		return -1;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	int seekFrom(int offset, SeekOrigin origin) override;

	bool isFinished() override
	{
		return finished;
	}

	int sectionIndex() const
	{
		return currentSectionIndex;
	}

	int recordIndex() const
	{
		auto section = getSection();
		return section ? section->recordIndex : -1;
	}

	/**
	 * @brief Get number of sections in this stream
	 */
	size_t count() const
	{
		return sectionCount;
	}

	/**
	 * @brief Get description of the current section
	 * @retval Section* The section information, or nullptr if there is no current section
	 */
	const Section* getSection() const
	{
		return getSection(currentSectionIndex);
	}

	/**
	 * @brief Get description for any section given its index
	 * @retval Section* The section information, or nullptr if section was not found
	 */
	const Section* getSection(unsigned index) const
	{
		if(index < sectionCount) {
			return &sections[index];
		} else {
			return nullptr;
		}
	}

	/**
	 * @brief Register a callback to be invoked when moving to a new section
	 */
	void onNextSection(NextSection callback)
	{
		nextSectionCallback = callback;
	}

	/**
	 * @brief Register a callback to be invoked when moving to a new record
	 */
	void onNextRecord(NextRecord callback)
	{
		nextRecordCallback = callback;
	}

	/**
	 * @brief Goto a new section immediately
	 */
	bool gotoSection(uint8_t index);

	/**
	 * @brief Goto a new section after current tag has been processed
	 */
	bool setNewSection(int8_t index)
	{
		if(index < 0 || index >= sectionCount) {
			return false;
		}
		newSection = index;
		return true;
	}

protected:
	/**
	 * @brief Invoked when moving to a new section
	 */
	virtual void nextSection();

	/**
	 * @brief Move to first/next record
	 * @retval bool true to emit section, false to skip
	 */
	virtual bool nextRecord()
	{
		if(nextRecordCallback) {
			return nextRecordCallback();
		}

		// By default, emit section once
		return recordIndex() < 0;
	}

private:
	void scanSource(uint8_t maxSections);

	IDataSourceStream* stream{nullptr};
	NextSection nextSectionCallback;
	NextRecord nextRecordCallback;
	String startTag;
	String endTag;
	std::unique_ptr<Section[]> sections;
	uint32_t readOffset{0};
	uint32_t sectionOffset{0};
	uint8_t sectionCount{0};
	int8_t currentSectionIndex{-1};
	int8_t newSection{-1};
	bool finished{false};
};
