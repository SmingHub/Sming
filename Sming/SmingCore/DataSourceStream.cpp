/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "../SmingCore/DataSourceStream.h"

MemoryDataStream::MemoryDataStream()
{
	buf = NULL;
	pos = NULL;
	size = 0;
	capacity = 0;
}

MemoryDataStream::~MemoryDataStream()
{
	free(buf);
	buf = NULL;
	pos = NULL;
	size = 0;
}

size_t MemoryDataStream::write(uint8_t charToWrite)
{
	return write(&charToWrite, 1);
}

size_t MemoryDataStream::write(const uint8_t* data, size_t len)
{
	//TODO: add queued buffers without full copy
	if (buf == NULL)
	{
		buf = (char*)malloc(len + 1);
		if (buf == NULL)
			return 0;
		buf[len] = '\0';
		memcpy(buf, data, len);
	}
	else
	{
		int cur = size;
		int required = cur + len + 1;
		if (required > capacity)
		{
			capacity = required < 256 ? required + 128 : required + 64;
			debugf("realloc %d -> %d", size, capacity);
			char * new_buf;
			//realloc can fail, store the result in temporary pointer
			new_buf = (char*)realloc(buf, capacity);

			if (new_buf == NULL)
			{
				return 0;
			}
			buf = new_buf;
		}
		buf[cur + len] = '\0';
		memcpy(buf + cur, data, len);
	}
	pos = buf;
	size += len;
	return len;
}

uint16_t MemoryDataStream::readMemoryBlock(char* data, int bufSize)
{
	int available = min(size - (pos - buf), bufSize);
	memcpy(data, pos, available);
	return available;
}

bool MemoryDataStream::seek(int len)
{
	if (len < 0) return false;

	pos += len;
	return true;
}

bool MemoryDataStream::isFinished()
{
	return size == (pos - buf);
}

///////////////////////////////////////////////////////////////////////////

FileStream::FileStream()
{
	handle = -1;
	size = -1;
	pos = 0;
}

FileStream::FileStream(String filename)
{
	attach(filename, eFO_ReadOnly);
}

bool FileStream::attach(String fileName, FileOpenFlags openFlags)
{
	handle = fileOpen(fileName.c_str(), openFlags);
	if (handle == -1)
	{
		debugf("File wasn't found: %s", fileName.c_str());
		size = -1;
		pos = 0;
		return false;
	}

	// Get size
	fileSeek(handle, 0, eSO_FileEnd);
	size = fileTell(handle);

	fileSeek(handle, 0, eSO_FileStart);
	pos = 0;

	debugf("attached file: %s (%d bytes)", fileName.c_str(), size);
	return true;
}

FileStream::~FileStream()
{
	fileClose(handle);
	handle = 0;
	pos = 0;
}

uint16_t FileStream::readMemoryBlock(char* data, int bufSize)
{
	int len = min(bufSize, size - pos);
	int available = fileRead(handle, data, len);
	fileSeek(handle, pos, eSO_FileStart); // Don't move cursor now (waiting seek)
	return available;
}

size_t FileStream::write(uint8_t charToWrite)
{
	uint8_t tempbuf[1]  {charToWrite};
	return write(tempbuf, 1);
}

size_t FileStream::write(const uint8_t *buffer, size_t size)
{
	if (!fileExist()) return 0;

	bool result = fileSeek(handle, 0, eSO_FileEnd);
	return fileWrite(handle, buffer, size);
}

bool FileStream::seek(int len)
{
	if (len < 0) return false;

	bool result = fileSeek(handle, len, eSO_CurrentPos) >= 0;
	if (result) pos += len;
	return result;
}

bool FileStream::isFinished()
{
	return fileIsEOF(handle);
}

String FileStream::fileName()
{
	spiffs_stat stat;
	fileStats(handle, &stat);
	return String((char*)stat.name);
}

bool FileStream::fileExist()
{
	return size != -1;
}

///////////////////////////////////////////////////////////////////////////

TemplateFileStream::TemplateFileStream(String templateFileName)
	: FileStream(templateFileName)
{
	state = eTES_Wait;
}

TemplateFileStream::~TemplateFileStream()
{
}

uint16_t TemplateFileStream::readMemoryBlock(char* data, int bufSize)
{
	debugf("READ Template (%d)", state);
	int available;

	if (state == eTES_StartVar)
	{
		if (templateData.contains(varName))
		{
			// Return variable value
			debugf("StartVar %s", varName.c_str());
			available = templateData[varName].length();
			memcpy(data, (char*)templateData[varName].c_str(), available);
			seek(skipBlockSize);
			varDataPos = 0;
			state = eTES_SendingVar;
			return available;
		}
		else
		{
			debugf("var %s not found", varName.c_str());
			state = eTES_Wait;
			int len = FileStream::readMemoryBlock(data, bufSize);
			return min(len, skipBlockSize);
		}
	}
	else if (state == eTES_SendingVar)
	{
		String *val = &templateData[varName];
		if (varDataPos < val->length())
		{
			debugf("continue TRANSFER variable value (not completed)");
			available = val->length() - varDataPos;
			memcpy(data, ((char*)val->c_str()) + varDataPos, available);
			return available;
		}
		else
		{
			debugf("continue to plaint text");
			state = eTES_Wait;
		}
	}

	int len = FileStream::readMemoryBlock(data, bufSize);
	char* tpl = data;
	if (tpl && len > 0)
	{
		char* end = tpl + len;
		char* cur = (char*)memchr(tpl, '{', len);
		char* lastFound = cur;
		while (cur != NULL)
		{
			lastFound = cur;
			char* p = cur + 1;
			for (; p < end; p++)
			{
				if (isspace(*p))
					break; // Not a var name
				else if (p - cur > TEMPLATE_MAX_VAR_NAME_LEN)
					break; // To long for var name
				else if (*p == '{')
					break; // New start..

				if (*p == '}')
				{
					int block = p - cur + 1;
					char varname[TEMPLATE_MAX_VAR_NAME_LEN + 1] = {0};
					memcpy(varname, cur + 1, p - cur - 1); // name without { and }
					varName = varname;
					state = eTES_Found;
					varWaitSize = cur - tpl;
					debugf("found var: %s, at %d (%d) - %d, send size %d", varName.c_str(), varWaitSize + 1, varWaitSize + getPos(), p - tpl, varWaitSize);
					skipBlockSize = block;
					if (varWaitSize == 0) state = eTES_StartVar;
					return varWaitSize; // return only plain text from template without our variable
				}
			}
			cur = (char*)memchr(p, '{', len - (p - tpl)); // continue searching..
		}
		if (lastFound != NULL && (lastFound - tpl) > (len - TEMPLATE_MAX_VAR_NAME_LEN))
		{
			debugf("trim end to %d from %d", lastFound - tpl, len);
			len = lastFound - tpl; // It can be a incomplete variable name. Don't split it!
		}
	}

	debugf("plain template text pos: %d, len: %d", getPos(), len);
	return len;
}

bool TemplateFileStream::seek(int len)
{
	if (len < 0) return false;
	//debugf("SEEK: %d, (%d)", len, state);

	if (state == eTES_Found)
	{
		//debugf("SEEK before Var: %d, (%d)", len, varWaitSize);
		varWaitSize -= len;
		if (varWaitSize == 0) state = eTES_StartVar;
	}
	else if (state == eTES_SendingVar)
	{
		varDataPos += len;
		return false; // not the end
	}

	return FileStream::seek(len);
}

void TemplateFileStream::setVar(String name, String value)
{
	templateData[name] = value;
}

// TODO: Remove that dependency from here ...
//void TemplateFileStream::setVarsFromRequest(const HttpRequest& request)
//{
//	if (request.requestGetParameters != NULL)
//		templateData.setMultiple(*request.requestGetParameters);
//	if (request.requestPostParameters != NULL)
//		templateData.setMultiple(*request.requestPostParameters);
//}

///////////////////////////////////////////////////////////////////////////

JsonObjectStream::JsonObjectStream()
	: rootNode(buffer.createObject()), send(true)
{
}

JsonObjectStream::~JsonObjectStream()
{
}

JsonObject& JsonObjectStream::getRoot()
{
	return rootNode;
}

uint16_t JsonObjectStream::readMemoryBlock(char* data, int bufSize)
{
	if (rootNode != JsonObject::invalid() && send)
	{
		int len = rootNode.printTo(*this);
		send = false;
	}

	return MemoryDataStream::readMemoryBlock(data, bufSize);
}

int JsonObjectStream::length()
{
	if (rootNode != JsonObject::invalid()) {
		return -1;
	}

	return rootNode.measureLength();
}
