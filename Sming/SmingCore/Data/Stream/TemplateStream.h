/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/* 	12/8/2018 (mikee47)
 *
 * 	The guts of TemplateFileStream moved to a generic TemplateStream which uses any
 * 	IDataSourceStream as input, not just a FileStream.
 */

#ifndef _SMING_CORE_DATA_TEMPLATESTREAM_H_
#define _SMING_CORE_DATA_TEMPLATESTREAM_H_

#include "DataSourceStream.h"
#include "WHashMap.h"
#include "WString.h"


#define TEMPLATE_MAX_VAR_NAME_LEN 16

/** @brief  Template variable (hash map) class
 *  @see    Wiring HashMap
 */
class TemplateVariables : public HashMap<String, String> {};

/** @brief  Template file stream expand state
 *  @ingroup constants
 *  @{
 */
enum TemplateExpandState {
	//
	eTES_Wait,
	//
	eTES_Found,
	// Variable name found in input stream
	eTES_StartVar,
	// Outputting variable content
	eTES_SendingVar
};
/** @} */



/** @brief Template stream class
 * 	@ingroup    stream data
 *  @{
 *
 */
class TemplateStream : public IDataSourceStream {
public:
	/** @brief Create a template file stream
     *  @param  templateFileName Template filename
     */
	TemplateStream(IDataSourceStream* stream) : _stream(stream)
	{}

	virtual ~TemplateStream()
	{
		delete _stream;
	}

	//Use base class documentation
	virtual StreamType getStreamType() const
	{
		return _stream ? eSST_Template : eSST_Invalid;
	}

	//Use base class documentation
	virtual size_t readMemoryBlock(char* data, size_t bufSize);

	//Use base class documentation
	virtual bool seek(int len);

	virtual bool isFinished()
	{
		return _stream ? _stream->isFinished() : true;
	}

	/** @brief  Set value of a variable in the template file
     *  @param  name Name of variable
     *  @param  value Value to assign to the variable
     *  @note   Sets and existing variable or adds a new variable if variable does not already exist
     */
	void setVar(const String& name, const String& value)
	{
		_templateData[name] = value;
	}

	/** @brief  Set multiple variables in the template file
     *  @param  vars Template Variables
     */
	void setVars(const TemplateVariables& vars)
	{
		_templateData.setMultiple(vars);
	}

	/** @brief  Get the template variables
     *  @retval TemplateVariables Reference to the template variables
     */
	inline TemplateVariables& variables()
	{
		return _templateData;
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
	IDataSourceStream* _stream = nullptr;
	TemplateVariables _templateData;
	TemplateExpandState _state = eTES_Wait;
	String _varName = nullptr;
	size_t _skipBlockSize = 0;
	size_t _varDataPos = 0;
	size_t _varWaitSize = 0;
};


/** @} */


#endif /* _SMING_CORE_DATA_DATASTREAM_H_ */
