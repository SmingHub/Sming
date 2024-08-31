/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Formatter.h
 *
 * @author mikee47 <mike@sillyhouse.net> Nov 2020
 *
 ****/

#pragma once

#include <WString.h>
#include <Data/WebConstants.h>
#include <Data/BitSet.h>

namespace Format
{
enum class Option {
	unicode, //< Use unicode escapes \uNNNN, otherwise hex \xNN
	utf8,	///< Convert extended ASCII to UTF8
	doublequote,
	singlequote,
	backslash,
};
using Options = BitSet<uint8_t, Option, 5>;

/**
 * @brief Escape standard control codes such as `\n` (below ASCII 0x20)
 * @param value String to be modified
 * @param options
 * @retval unsigned Number of control characters found and replaced
 */
unsigned escapeControls(String& value, Options options);

/**
 * @brief Virtual class to perform format-specific String adjustments
 */
class Formatter
{
public:
	/**
	 * @brief Perform any necessary text escaping so output is valid
	 */
	virtual void escape(String& value) const = 0;

	/**
	 * @brief Convert a value into quoted string
	 */
	virtual void quote(String& value) const = 0;

	/**
	 * @brief Remove any quotes from a value
	 */
	virtual void unQuote(String& value) const = 0;

	/**
	 * @brief Corresponding MIME type for this format
	 * @note New types must be added to WebConstants.h
	 */
	virtual MimeType mimeType() const = 0;
};

} // namespace Format
