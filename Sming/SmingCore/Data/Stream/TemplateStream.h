/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_TEMPLATE_FILE_STREAM_H_
#define _SMING_CORE_DATA_TEMPLATE_FILE_STREAM_H_

#include "FileStream.h"
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

/// Template file stream class
class TemplateFileStream : public FileStream
{
public:
	/** @brief Create a template file stream
     *  @param  templateFileName Template filename
     */
	TemplateFileStream(const String& templateFileName);
	virtual ~TemplateFileStream();

	//Use base class documentation
	virtual StreamType getStreamType()
	{
		return eSST_TemplateFile;
	}

	//Use base class documentation
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

	//Use base class documentation
	virtual bool seek(int len);

	/** @brief  Set value of a variable in the template file
     *  @param  name Name of variable
     *  @param  value Value to assign to the variable
     *  @note   Sets and existing variable or adds a new variable if variable does not already exist
     */
	void setVar(String name, String value);

	/** @brief  Set multiple variables in the template file
     *  @param  vars Template Variables
     */
	void setVars(const TemplateVariables& vars);

	/** @brief  Get the template variables
     *  @retval TemplateVariables Reference to the template variables
     */
	inline TemplateVariables& variables()
	{
		return templateData;
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int available()
	{
		return -1;
	}

private:
	TemplateVariables templateData;
	TemplateExpandState state;
	String varName;
	int skipBlockSize = 0;
	int varDataPos = 0;
	int varWaitSize = 0;
};

/** @} */
#endif /* _SMING_CORE_DATA_TEMPLATE_FILE_STREAM_H_ */
