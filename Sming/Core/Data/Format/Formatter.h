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
#include <Network/WebConstants.h>

namespace Format
{
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
	 * @brief Correspdoning MIME type for this format
	 * @note New types must be added to WebConstants.h
	 */
	virtual MimeType mimeType() const = 0;
};

} // namespace Format
