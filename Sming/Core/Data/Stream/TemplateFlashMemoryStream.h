/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TemplateFlashMemoryStream.h
 *
 * @author: 23 Oct 2018 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "FlashMemoryStream.h"
#include "TemplateStream.h"

/**
  * @brief      Template Flash memory stream class
  * @ingroup    stream data
  *
  *  @{
 */

class TemplateFlashMemoryStream : public TemplateStream
{
public:
	/** @brief Create a template stream on top of a flash memory stream
     *  @param  flashString Source data for the stream
     */
	TemplateFlashMemoryStream(const FlashString& flashString) : TemplateStream(new FlashMemoryStream(flashString))
	{
	}
};

/** @} */
