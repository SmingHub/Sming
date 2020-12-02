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

/**
 * @brief Presents each section within a source stream as a separate stream
 *
 * Sections are marked {!SECTION} ... {/SECTION}
 * This is typically used with templating but can be used with any stream type provided
 * the tags do not conflict with content.
 *
 * TODO: Allow tags to be changed.
 */
class SectionStream : public IDataSourceStream
{
public:
	static constexpr uint8_t maxSections = 5;

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

	SectionStream(IDataSourceStream* source) : stream(source), startTag(F("{SECTION}")), endTag(F("{/SECTION}"))
	{
		scanSource();
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

	size_t count() const
	{
		return sectionCount;
	}

	const Section* getSection() const
	{
		return getSection(currentSectionIndex);
	}

	const Section* getSection(unsigned index) const
	{
		if(index >= 0 && index < sectionCount) {
			return &sections[index];
		} else {
			return nullptr;
		}
	}

	void onNextSection(NextSection callback)
	{
		nextSectionCallback = callback;
	}

	void onNextRecord(NextRecord callback)
	{
		nextRecordCallback = callback;
	}

	/**
	 * @brief Goto a new section immediately
	 */
	bool gotoSection(uint8_t index);

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
	void scanSource();

	IDataSourceStream* stream{nullptr};
	NextSection nextSectionCallback;
	NextRecord nextRecordCallback;
	String startTag;
	String endTag;
	Section sections[maxSections]{};
	uint32_t readOffset{0};
	uint32_t sectionOffset{0};
	uint8_t sectionCount{0};
	int8_t currentSectionIndex{-1};
	int8_t newSection{-1};
	bool finished{false};
};
