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
	using GetValueDelegate = Delegate<String(const char* name)>;

	/** @brief Create a template stream
     *  @param stream source of template data
     */
	TemplateStream(IDataSourceStream* stream) : stream(stream)
	{
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
	 * @brief Set a callback to obtain variable values
	 * @param callback Invoked only if variable name not found in map
	 */
	void onGetValue(GetValueDelegate callback)
	{
		getValueCallback = callback;
	}

protected:
	String getValue(const char* name);

private:
	IDataSourceStream* stream = nullptr;
	TemplateVariables templateData;
	GetValueDelegate getValueCallback;
	TemplateExpandState state = eTES_Wait;
	String value;
	size_t skipBlockSize = 0;
	size_t varDataPos = 0;
	size_t varWaitSize = 0;
};

/**
 * @deprecated Use `TemplateStream::Variables` instead
 */
typedef TemplateStream::Variables TemplateVariables;
