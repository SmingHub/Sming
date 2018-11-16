/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 23 Oct 2018 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#ifndef _SMING_CORE_DATA_TEMPLATE_FLASH_MEMORY_STREAM_H_
#define _SMING_CORE_DATA_TEMPLATE_FLASH_MEMORY_STREAM_H_

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

#endif /* _SMING_CORE_DATA_TEMPLATE_FLASH_MEMORY_STREAM_H_ */
