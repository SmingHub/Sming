/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATASTREAM_H_
#define _SMING_CORE_DATASTREAM_H_

#include <user_config.h>
#include "../SmingCore/FileSystem.h"
#include "../Services/ArduinoJson/include/ArduinoJson.h"
#include "../Wiring/WString.h"
#include "../Wiring/WHashMap.h"

#define TEMPLATE_MAX_VAR_NAME_LEN	16

class HttpRequest;

/** @brief  Data stream type
 *  @ingroup constants
 *  @{
 */
enum StreamType
{
	eSST_Memory, ///< Memory data stream
	eSST_File, ///< File data stream
	eSST_TemplateFile, ///< Template file data stream
	eSST_JsonObject, ///< JSON object data stream
	eSST_User, ///< User defined data stream
	eSST_Unknown ///< Unknown data stream type
};
/** @} */

/** @defgroup   stream Stream functions
 *  @brief      Data stream classes
 *  @{
*/

/** @brief  Template variable (hash map) class
 *  @see    Wiring HashMap
 */
class TemplateVariables : public HashMap<String, String>
{
};

///Base class for data source stream
class IDataSourceStream
{
public:
	virtual ~IDataSourceStream() {}

    /** @brief  Get the stream type
     *  @retval StreamType The stream type.
     *  @todo   Return value of IDataSourceStream:getStreamType base class function should be of type StreamType, e.g. eSST_User
     */
	virtual StreamType getStreamType() = 0;

    /** @brief  Read a block of memory
     *  @param  data Pointer to the data to be read
     *  @param  bufSize Quantity of chars to read
     *  @retval uint16_t Quantity of chars read
     *  @todo   Should IDataSourceStream::readMemoryBlock return same data type as its bufSize param?
     */
	virtual uint16_t readMemoryBlock(char* data, int bufSize) = 0;

	/** @brief  Move read cursor
	 *  @param  len Position within stream to move cursor to
	 *  @retval bool True on success.
	 */
	virtual bool seek(int len) = 0;

    /** @brief  Check if stream is finished
     *  @retval bool True on success.
     */
	virtual bool isFinished() = 0;

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	virtual int length() {  return -1; }
};

/// Memory data stream class
class MemoryDataStream : public Print, public IDataSourceStream
{
public:
    /** @brief Memory data stream base class
    */
	MemoryDataStream();
	virtual ~MemoryDataStream();

    //Use base class documentation
	virtual StreamType getStreamType() { return eSST_Memory; }

	/** @brief  Get a pointer to the current position
	 *  @retval "const char*" Pointer to current cursor position within the data stream
	 */
	const char* getStreamPointer() { return pos; }

	/** @brief  Get size of stream
	 *  @retval int Quantity of chars in stream
	 *
	 *  @deprecated Use getLength instead
	 */
	int getStreamLength() { return size; }

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	*/
	int length() { return size; }

    /** @brief  Write a single char to stream
     *  @param  charToWrite Char to write to the stream
     *  @retval size_t Quantity of chars written to stream (always 1)
     */
	virtual size_t write(uint8_t charToWrite);

    /** @brief  Write chars to stream
     *  @param  buffer Pointer to buffer to write to the stream
     *  @param  size Quantity of chars to writen
     *  @retval size_t Quantity of chars written to stream
     */
	virtual size_t write(const uint8_t *buffer, size_t size);

    //Use base class documentation
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

    //Use base class documentation
	virtual bool seek(int len);

    //Use base class documentation
	virtual bool isFinished();

private:
	char* buf;
	char* pos;
	int size;
	int capacity;
};

/// File stream class
class FileStream : public IDataSourceStream
{
public:
	
    /** @brief  Create a file stream
     *  @param  fileName Name of file to open
     */
	FileStream();
	FileStream(const String& fileName);
	virtual ~FileStream();

	virtual bool attach(const String& fileName, FileOpenFlags openFlags);
    //Use base class documentation
	virtual StreamType getStreamType() { return eSST_File; }

	virtual size_t write(uint8_t charToWrite);
	virtual size_t write(const uint8_t *buffer, size_t size);

    //Use base class documentation
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

    //Use base class documentation
	virtual bool seek(int len);

    //Use base class documentation
	virtual bool isFinished();

	String fileName(); ///< Filename of file stream is attached to
	bool fileExist(); ///< True if file exists

    /** @brief  Get the offset of cursor from beginning of data
     *  @retval int Cursor offset
     */
	inline int getPos() { return pos; }

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int length() { return -1; }

private:
	file_t handle;
	int pos;
	int size;
};
/** @} */

/** @brief  Template file stream expand state
 *  @ingroup constants
 *  @{
 */
enum TemplateExpandState
{
	eTES_Wait, ///< Template expand state wait
	eTES_Found, ///< Template expand state found
	eTES_StartVar, ///< Template expand state start variable
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
	virtual StreamType getStreamType() { return eSST_TemplateFile; }

    //Use base class documentation
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

    //Use base class documentation
	virtual bool seek(int len);

    /** @brief  Set value of a variable in the template file
     *  @param  name Name of variable
     *  @param  value Value to assign to the variable
     *  @note   Sets and existing varible or adds a new variable if variable does not already exist
     */
	void setVar(String name, String value);

    /** @brief  Set the value of variables from the content of a HTTP request
     *  @param  request HTTP request
     */
	void setVarsFromRequest(const HttpRequest& request);

    /** @brief  Get the template variables
     *  @retval TemplateVariables Reference to the template variables
     */
	inline TemplateVariables& variables() { return templateData; }

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int length() { return -1; }

private:
	TemplateVariables templateData;
	TemplateExpandState state;
	String varName;
	int skipBlockSize = 0;
	int varDataPos = 0;
	int varWaitSize = 0;
};

///JSON object stream class
class JsonObjectStream : public MemoryDataStream
{
public:
    /** @brief  Create a JSON object stream
    */
	JsonObjectStream();
	virtual ~JsonObjectStream();

    //Use base class documentation
	virtual StreamType getStreamType() { return eSST_JsonObject; }

    /** @brief  Get the JSON root node
     *  @retval JsonObject Reference to the root node
     */
	JsonObject& getRoot();

    //Use base class documentation
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	 */
	int length();

private:
	DynamicJsonBuffer buffer;
	JsonObject &rootNode;
	bool send;
};

/** @} */
#endif /* _SMING_CORE_DATASTREAM_H_ */
