/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TemplateStream.h
 *
 ****/

#pragma once

#include "DataSourceStream.h"
#include "WHashMap.h"
#include "WString.h"

#define TEMPLATE_MAX_VAR_NAME_LEN 16

/** @brief  Template variable (hash map) class
 *  @see    Wiring HashMap
 */
class TemplateVariables : public HashMap<String, String>
{
};

/** @brief  Template file stream expand state
 *  @ingroup constants
 *  @{
 */
enum TemplateExpandState {
	eTES_Wait,		///< Template expand state wait
	eTES_Found,		///< Template expand state found
	eTES_StartVar,  ///< Template expand state start variable
	eTES_SendingVar ///< Template expand state sending variable
};
/** @} */

/** @addtogroup stream
 *  @{
 */

class TemplateStream : public IDataSourceStream
{
public:
	/** @brief Create a template stream
     *  @param stream source of template data
     */
	TemplateStream(IDataSourceStream* stream) : stream(stream)
	{
		// Pre-allocate string to maximum length
		varName.reserve(TEMPLATE_MAX_VAR_NAME_LEN);
	}

	~TemplateStream()
	{
		delete stream;
	}

	//Use base class documentation
	StreamType getStreamType() const override
	{
		return stream ? eSST_Template : eSST_Invalid;
	}

	//Use base class documentation
	uint16_t readMemoryBlock(char* data, int bufSize) override;

	//Use base class documentation
	bool seek(int len) override;

	bool isFinished() override
	{
		return stream ? stream->isFinished() : true;
	}

	/** @brief  Set value of a variable in the template file
     *  @param  name Name of variable
     *  @param  value Value to assign to the variable
     *  @note   Sets and existing variable or adds a new variable if variable does not already exist
     */
	void setVar(const String& name, const String& value)
	{
		templateData[name] = value;
	}

	/** @brief  Set multiple variables in the template file
     *  @param  vars Template Variables
     */
	void setVars(const TemplateVariables& vars)
	{
		templateData.setMultiple(vars);
	}

	/** @brief  Get the template variables
     *  @retval TemplateVariables Reference to the template variables
     */
	inline TemplateVariables& variables()
	{
		return templateData;
	}

	String getName() const override
	{
		return stream ? stream->getName() : nullptr;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 *
	 * We cannot reliably determine available size so use default method which returns -1.
	 *
	 * 	int available()
	 */

private:
	IDataSourceStream* stream = nullptr;
	TemplateVariables templateData;
	TemplateExpandState state = eTES_Wait;
	String varName;
	size_t skipBlockSize = 0;
	size_t varDataPos = 0;
	size_t varWaitSize = 0;
};

/** @} */
