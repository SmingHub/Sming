/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SectionTemplate.h
 *
 * @author mikee47 <mike@sillyhouse.net> Nov 2020
 *
 ****/

#pragma once

#include "SectionStream.h"
#include "TemplateStream.h"
#include <Data/BitSet.h>
#include "../Format.h"

/**
 * @brief Command map
 *
 * Commands may have zero or more arguments, which may be:
 * 		Number:		starts with a digit
 * 		String:		"..."
 * 		expression: {...}
 *
 * Anything else is treated as a variable name.
 * Separator is :
 *
 * @note Command tags are prefixed with _ to allow use of reserved words
 * in the Command enumeration. This represents the ! prefix in actual use.
 */
#define SECTION_TEMPLATE_COMMAND_MAP(XX)                                                                               \
	XX(_as_int, "{!int:A} Output A as integer")                                                                        \
	XX(_as_float, "{!float:A} Output A as float")                                                                      \
	XX(_as_string, "{!string:A} Output A as quoted string")                                                            \
	XX(_mime_type, "{!mime_type:A} Get MIME type string for a filename")                                               \
	XX(_replace, "{!replace:A:B:C} Copy of A with all occurrences of B replaced with C")                               \
	XX(_length, "{!length:A} Number of characters in A")                                                               \
	XX(_pad,                                                                                                           \
	   "{!pad:A:B:C} Copy of A padded to at least B characters with C (default is space). Use -ve B to left-pad. C")   \
	XX(_repeat, "{!repeat:A:B} Repeat A, number of iterations is B")                                                   \
	XX(_kb, "{!kb:A} Convert A to KB")                                                                                 \
	XX(_ifdef, "{!ifdef:A}block{/if} emit block if A is not zero-length")                                              \
	XX(_ifndef, "{!ifdef:A}block{/if} emit block if A is zero-length")                                                 \
	XX(_ifeq, "{!ifeq:A:B} emit block if A == B")                                                                      \
	XX(_ifneq, "{!ifneq:A:B} emit block if A != B")                                                                    \
	XX(_ifgt, "{!ifgt:A:B} emit block if A > B")                                                                       \
	XX(_iflt, "{!iflt:A:B} emit block if A < B")                                                                       \
	XX(_ifge, "{!ifge:A:B} emit block if A >= B")                                                                      \
	XX(_ifle, "{!ifle:A:B} emit block if A <= B")                                                                      \
	XX(_ifbtw, "{!ifbtw:A:B:C} emit block if B <= A <= C")                                                             \
	XX(_ifin, "{!ifin:A:B} emit block if A contains B")                                                                \
	XX(_ifnin, "{!ifin:A:B} emit block if A does not contain B")                                                       \
	XX(_else, "{!else}")                                                                                               \
	XX(_endif, "{!endif}")                                                                                             \
	XX(_add, "{!add:A:B} A - B")                                                                                       \
	XX(_sub, "{!sub:A:B} A - B")                                                                                       \
	XX(_goto, "{!goto:A} move to section A")                                                                           \
	XX(_count, "{!count:A} emit number of records in section A")                                                       \
	XX(_index, "{!index:A} emit current record index for section A")

#define SECTION_TEMPLATE_FIELD_MAP(XX)                                                                                 \
	XX(section, "{$section} Current section index")                                                                    \
	XX(record, "{$record} Current record index")

/**
 * @brief Provides enhanced template tag processing for use with a SectionStream
*/
class SectionTemplate : public TemplateStream
{
public:
	enum class Command {
		_unknown = 0,
#define XX(name, comment) name,
		SECTION_TEMPLATE_COMMAND_MAP(XX)
#undef XX
	};

	enum class Field {
		unknown = 0,
#define XX(name, comment) name,
		SECTION_TEMPLATE_FIELD_MAP(XX)
#undef XX
	};

	SectionTemplate(IDataSourceStream* source);

	/**
	 * @brief Application callback to process additional fields
	 * @param templateStream
	 * @param name Field name, never null
	 * @retval String The field value
	 * @note Applications should call `escape()` if required before returning content.
	 * Use `stream.state()` to determine the current section being processed.
	 */
	using GetValue = Delegate<String(const char* name)>;

	using NextRecord = SectionStream::NextRecord;

	void onGetValue(GetValue callback)
	{
		getValueCallback = callback;
	}

	void setFormatter(Formatter& formatter)
	{
		activeFormatter = &formatter;
	}

	Formatter& formatter() const
	{
		return *activeFormatter;
	}

	MimeType getMimeType() const override
	{
		return activeFormatter->mimeType();
	}

	const SectionStream& stream() const
	{
		return sectionStream;
	}

	int sectionIndex() const
	{
		return sectionStream.sectionIndex();
	}

	uint8_t sectionCount() const
	{
		return sectionStream.count();
	}

	int recordIndex() const
	{
		return sectionStream.recordIndex();
	}

	bool gotoSection(uint8_t index);

	void onNextRecord(NextRecord callback)
	{
		nextRecordCallback = callback;
	}

	String evaluate(char*& expr) override;
	String getValue(const char* name) override;

protected:
	/**
	 * @brief Move to next record
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
	String openTag(bool enable);
	String closeTag();
	String elseTag();

	SectionStream sectionStream;
	Formatter* activeFormatter;
	GetValue getValueCallback;
	NextRecord nextRecordCallback;
	BitSet32 conditionalFlags; // Enable state for each level
	uint8_t conditionalLevel{0};
	int8_t newSection{-1};
};
