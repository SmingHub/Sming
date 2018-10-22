/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_TEMPLATE_FILE_STREAM_H_
#define _SMING_CORE_DATA_TEMPLATE_FILE_STREAM_H_

#include "FileStream.h"
#include "TemplateStream.h"

/**
  * @brief      Template File stream class
  * @ingroup    stream data
  *
  *  @{
 */

class TemplateFileStream : public TemplateStream
{
public:
	/** @brief Create a template file stream
     *  @param  templateFileName Template filename
     */
	TemplateFileStream(const String& fileName) : TemplateStream(new FileStream(fileName))
	{
	}
};

/** @} */

#endif /* _SMING_CORE_DATA_TEMPLATE_FILE_STREAM_H_ */
