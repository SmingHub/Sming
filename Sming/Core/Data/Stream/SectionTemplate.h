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
 * Conditional if/else/endif statements may be nested.
 *
 * @note Command tags are prefixed with 'Q' to allow use of reserved words
 * in the Command enumeration. This represents the ! prefix in actual use.
 */
#define SECTION_TEMPLATE_COMMAND_MAP(XX)                                                                               \
	XX(Qas_int, "{!int:A} Output A as integer")                                                                        \
	XX(Qas_float, "{!float:A} Output A as float")                                                                      \
	XX(Qas_string, "{!string:A} Output A as quoted string")                                                            \
	XX(Qmime_type, "{!mime_type:A} Get MIME type string for a filename")                                               \
	XX(Qreplace, "{!replace:A:B:C} Copy of A with all occurrences of B replaced with C")                               \
	XX(Qlength, "{!length:A} Number of characters in A")                                                               \
	XX(Qpad,                                                                                                           \
	   "{!pad:A:B:C} Copy of A padded to at least B characters with C (default is space). Use -ve B to left-pad. C")   \
	XX(Qrepeat, "{!repeat:A:B} Repeat A, number of iterations is B")                                                   \
	XX(Qkb, "{!kb:A} Convert A to KB")                                                                                 \
	XX(Qifdef, "{!ifdef:A} emit block if A is not zero-length")                                                        \
	XX(Qifndef, "{!ifdef:A} emit block if A is zero-length")                                                           \
	XX(Qifeq, "{!ifeq:A:B} emit block if A == B")                                                                      \
	XX(Qifneq, "{!ifneq:A:B} emit block if A != B")                                                                    \
	XX(Qifgt, "{!ifgt:A:B} emit block if A > B")                                                                       \
	XX(Qiflt, "{!iflt:A:B} emit block if A < B")                                                                       \
	XX(Qifge, "{!ifge:A:B} emit block if A >= B")                                                                      \
	XX(Qifle, "{!ifle:A:B} emit block if A <= B")                                                                      \
	XX(Qifbtw, "{!ifbtw:A:B:C} emit block if B <= A <= C")                                                             \
	XX(Qifin, "{!ifin:A:B} emit block if A contains B")                                                                \
	XX(Qifnin, "{!ifin:A:B} emit block if A does not contain B")                                                       \
	XX(Qelse, "{!else}")                                                                                               \
	XX(Qendif, "{!endif}")                                                                                             \
	XX(Qadd, "{!add:A:B} A + B")                                                                                       \
	XX(Qsub, "{!sub:A:B} A - B")                                                                                       \
	XX(Qgoto, "{!goto:A} move to section A")                                                                           \
	XX(Qcount, "{!count:A} emit number of records in section A")                                                       \
	XX(Qindex, "{!index:A} emit current record index for section A")

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
		Qunknown = 0,
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

	SectionTemplate(IDataSourceStream* source, uint8_t maxSections = 5);

	/**
	 * @brief Application callback to process additional fields
	 * @param templateStream
	 * @param name Field name, never null
	 * @retval String The field value
	 * @note Applications should call `escape()` if required before returning content.
	 */
	using GetValue = Delegate<String(const char* name)>;

	using NextRecord = SectionStream::NextRecord;

	/**
	 * @brief Set a callback to be invoked
	 *
	 * Alternative to subclassing.
	 */
	void onGetValue(GetValue callback)
	{
		getValueCallback = callback;
	}

	/**
	 * @brief Associate a text format with this template stream
	 * @param formatter Provide formatter so we can call escape(), etc. as required
	 */
	void setFormatter(Formatter& formatter)
	{
		activeFormatter = &formatter;
	}

	/**
	 * @brief Get the stream format
	 * @retval Formatter& The formatter in effect. Default is :cpp:class:Format::Standard.
	 */
	Formatter& formatter() const
	{
		return *activeFormatter;
	}

	/**
	 * @brief Get the MIME type associated with this template stream
	 * @retval MimeType As defined by the formatter. Default is MIME_TEXT.
	 */
	MimeType getMimeType() const override
	{
		return activeFormatter->mimeType();
	}

	/**
	 * @brief Access the underlying section stream
	 * @retval SectionStream& Wraps source stream provided in constructor
	 * 
	 * Provided for debugging and other purposes.
	 * Applications should not use this method.
	 */
	const SectionStream& stream() const
	{
		return sectionStream;
	}

	/**
	 * @brief Get the index for the current section
	 * @retval int Indices are 0-based, returns -1 if 'Before Start'
	 */
	int sectionIndex() const
	{
		return sectionStream.sectionIndex();
	}

	/**
	 * @brief Get number of sections in source stream
	 * @retval uint8_t Source is scanned in constructor so this is always valid
	 */
	uint8_t sectionCount() const
	{
		return sectionStream.count();
	}

	/**
	 * @brief Get current record index
	 * @retval int Indices are 0-based, returns -1 if 'Before Start'
	 */
	int recordIndex() const
	{
		return sectionStream.recordIndex();
	}

	/**
	 * @brief Discard current output and change current section
	 * @param uint8_t Index of section to move to
	 * @retval bool true on success, false if section index invalid
	 */
	bool gotoSection(uint8_t index);

	/**
	 * @brief Set a callback to be invoked when a new record is required
	 *
	 * Can be used as alternative to subclassing.
	 */
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
	Formatter* activeFormatter{&Format::standard};
	GetValue getValueCallback;
	NextRecord nextRecordCallback;
	BitSet32 conditionalFlags; // Enable state for each level
	uint8_t conditionalLevel{0};
	int8_t newSection{-1};
};
