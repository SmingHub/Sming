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

enum StreamType
{
	eSST_Memory,
	eSST_File,
	eSST_TepmplateFile,
	eSST_JsonObject,
	eSST_User,
	eSST_Unknown
};

class TemplateVariables : public HashMap<String, String>
{
};

class IDataSourceStream
{
public:
	virtual ~IDataSourceStream() {}

	virtual StreamType getStreamType() = 0;

	virtual uint16_t readMemoryBlock(char* data, int bufSize) = 0;
	virtual bool seek(int len) = 0;
	virtual bool isFinished() = 0;
};

class MemoryDataStream : public Print, public IDataSourceStream
{
public:
	MemoryDataStream();
	virtual ~MemoryDataStream();

	virtual StreamType getStreamType() { return eSST_Memory; }
	const char* getStreamPointer() { return pos; }
	int getStreamLength() { return size; }

	virtual size_t write(uint8_t charToWrite);
	virtual size_t write(const uint8_t *buffer, size_t size);

	virtual uint16_t readMemoryBlock(char* data, int bufSize);
	virtual bool seek(int len);
	virtual bool isFinished();

private:
	char* buf;
	char* pos;
	int size;
	int capacity;
};

class FileStream : public IDataSourceStream
{
public:
	FileStream(String fileName);
	virtual ~FileStream();

	virtual StreamType getStreamType() { return eSST_File; }

	virtual uint16_t readMemoryBlock(char* data, int bufSize);
	virtual bool seek(int len);
	virtual bool isFinished();

	String fileName();
	bool fileExist();
	inline int getPos() { return pos; }

private:
	file_t handle;
	int pos;
	int size;
};

enum TemplateExpandState
{
	eTES_Wait,
	eTES_Found,
	eTES_StartVar,
	eTES_SendingVar
};

class TemplateFileStream : public FileStream
{
public:
	TemplateFileStream(String templateFileName);
	virtual ~TemplateFileStream();

	virtual StreamType getStreamType() { return eSST_TepmplateFile; }

	virtual uint16_t readMemoryBlock(char* data, int bufSize);
	virtual bool seek(int len);

	void setVar(String name, String value);
	void setVarsFromRequest(const HttpRequest& request);
	inline TemplateVariables& variables() { return templateData; }

private:
	TemplateVariables templateData;
	TemplateExpandState state;
	String varName;
	int skipBlockSize;
	int varDataPos;
	int varWaitSize;
};

class JsonObjectStream : public MemoryDataStream
{
public:
	JsonObjectStream();
	virtual ~JsonObjectStream();

	virtual StreamType getStreamType() { return eSST_JsonObject; }

	JsonObject& getRoot();

	virtual uint16_t readMemoryBlock(char* data, int bufSize);

private:
	DynamicJsonBuffer buffer;
	JsonObject &rootNode;
	bool send;
};


#endif /* _SMING_CORE_DATASTREAM_H_ */
