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

#ifndef TEMPLATE_MAX_VAR_NAME_LEN
/**
 * @brief Maximum length of a template variable name
 * @see See `TemplateStream`
 */
#define TEMPLATE_MAX_VAR_NAME_LEN 32
#endif

/**
 * @brief Stream which performs variable-value substitution on-the-fly
 *
 * Template uses {varname} style markers which are replaced as the stream is read.
 * 
 * Note: There must be no space after the opening brace.
 * For example, `{ varname }` will be emitted as-is without modification.
 *
 * @ingroup stream
 */
class TemplateStream : public IDataSourceStream
{
public:
	using Variables = HashMap<String, String>;
	using GetValueDelegate = Delegate<String(const char* name)>;

	/** @brief Create a template stream
     *  @param stream source of template data
     *  @param owned If true (default) then stream will be destroyed when complete
     */
	TemplateStream(IDataSourceStream* stream, bool owned = true)
		: stream(stream), streamOwned(owned), doubleBraces(false)

	{
		reset();
	}

	~TemplateStream()
	{
		if(streamOwned) {
			delete stream;
		}
	}

	StreamType getStreamType() const override
	{
		return stream ? eSST_Template : eSST_Invalid;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	int seekFrom(int offset, SeekOrigin origin) override;

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
	void setVars(const Variables& vars)
	{
		templateData.setMultiple(vars);
	}

	/** @brief  Get the template variables
     *  @retval TemplateVariables Reference to the template variables
     */
	Variables& variables()
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

	/**
	 * @brief During processing applications may suppress output of certain sections
	 * by calling this method from within the getValue callback
	 */
	void enableOutput(bool enable)
	{
		enableNextState = enable;
	}

	bool isOutputEnabled() const
	{
		return outputEnabled;
	}

	/**
	 * @brief Use two braces {{X}} to mark tags
	 * @param enable true: use two braces, false (default): single brace only
	 */
	void setDoubleBraces(bool enable)
	{
		doubleBraces = enable;
	}

	virtual String evaluate(char*& expr);

	/**
	 * @brief Fetch a templated value
	 * @param name The variable name
	 * @retval String value, invalid to emit tag unprocessed
	 */
	virtual String getValue(const char* name);

private:
	void reset()
	{
		value = nullptr;
		streamPos = 0;
		valuePos = 0;
		valueWaitSize = 0;
		tagLength = 0;
		sendingValue = false;
		outputEnabled = true;
		enableNextState = true;
	}

	IDataSourceStream* stream;
	Variables templateData;
	GetValueDelegate getValueCallback;
	String value;
	uint32_t streamPos;		///< Position in output stream
	uint16_t valuePos;		///< How much of variable value has been sent
	uint16_t valueWaitSize; ///< Chars to send before variable value
	uint8_t tagLength;
	bool streamOwned : 1;
	bool sendingValue : 1;
	bool outputEnabled : 1;
	bool enableNextState : 1;
	bool doubleBraces : 1;
};
