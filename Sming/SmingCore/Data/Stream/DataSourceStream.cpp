/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "Data/Stream/DataSourceStream.h"

#include <algorithm>

int IDataSourceStream::read()
{
	int res = peek();
	if(res != -1) {
		seek(1);
	}

	return res;
}

int IDataSourceStream::peek()
{
	char c;
	if(readMemoryBlock(&c, 1) == 1) {
		return (int)c;
	}

	return -1;
}

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

int MemoryDataStream::available()
{
	return size - (pos - buf);
}

size_t MemoryDataStream::write(uint8_t charToWrite)
{
	return write(&charToWrite, 1);
}

size_t MemoryDataStream::write(const uint8_t* data, size_t len)
{
	//TODO: add queued buffers without full copy
	if(buf == NULL) {
		buf = (char*)malloc(len + 1);
		if(buf == NULL)
			return 0;
		buf[len] = '\0';
		memcpy(buf, data, len);
	} else {
		int cur = size;
		int required = cur + len + 1;
		if(required > capacity) {
			capacity = required < 256 ? required + 128 : required + 64;
			debug_d("realloc %d -> %d", size, capacity);
			char* new_buf;
			//realloc can fail, store the result in temporary pointer
			new_buf = (char*)realloc(buf, capacity);

			if(new_buf == NULL) {
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
	int available = std::min(size - (pos - buf), bufSize);
	memcpy(data, pos, available);
	return available;
}

bool MemoryDataStream::seek(int len)
{
	if(len < 0)
		return false;

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

FileStream::FileStream(const String& filename)
{
	attach(filename, eFO_ReadOnly);
}

bool FileStream::attach(const String& fileName, FileOpenFlags openFlags)
{
	handle = fileOpen(fileName.c_str(), openFlags);
	if(handle == -1) {
		debug_w("File wasn't found: %s", fileName.c_str());
		size = -1;
		pos = 0;
		return false;
	}

	// Get size
	fileSeek(handle, 0, eSO_FileEnd);
	size = fileTell(handle);

	fileSeek(handle, 0, eSO_FileStart);
	pos = 0;

	debug_d("attached file: %s (%d bytes)", fileName.c_str(), size);
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
	int len = std::min(bufSize, size - pos);
	int available = fileRead(handle, data, len);
	fileSeek(handle, pos, eSO_FileStart); // Don't move cursor now (waiting seek)
	if(available < 0) {
		available = 0;
	}
	return available;
}

size_t FileStream::write(uint8_t charToWrite)
{
	uint8_t tempbuf[1]{charToWrite};
	return write(tempbuf, 1);
}

size_t FileStream::write(const uint8_t* buffer, size_t size)
{
	if(!fileExist())
		return 0;

	bool result = fileSeek(handle, 0, eSO_FileEnd);
	return fileWrite(handle, buffer, size);
}

bool FileStream::seek(int len)
{
	if(len < 0)
		return false;

	bool result = fileSeek(handle, len, eSO_CurrentPos) >= 0;
	if(result)
		pos += len;
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

String FileStream::id()
{
	spiffs_stat stat;
	fileStats(handle, &stat);

#define ETAG_SIZE 16
	char buf[ETAG_SIZE];
	m_snprintf(buf, ETAG_SIZE, _F("00f-%x-%x0-%x"), stat.obj_id, stat.size, strlen((char*)stat.name));

	return String(buf);
}

///////////////////////////////////////////////////////////////////////////

TemplateFileStream::TemplateFileStream(const String& templateFileName) : FileStream(templateFileName)
{
	state = eTES_Wait;
}

TemplateFileStream::~TemplateFileStream()
{
}

uint16_t TemplateFileStream::readMemoryBlock(char* data, int bufSize)
{
	debug_d("READ Template (%d)", state);
	int available;

	if(state == eTES_StartVar) {
		if(templateData.contains(varName)) {
			// Return variable value
			debug_d("StartVar %s", varName.c_str());
			available = templateData[varName].length();
			memcpy(data, (char*)templateData[varName].c_str(), available);
			seek(skipBlockSize);
			varDataPos = 0;
			state = eTES_SendingVar;
			return available;
		} else {
			debug_d("var %s not found", varName.c_str());
			state = eTES_Wait;
			int len = FileStream::readMemoryBlock(data, bufSize);
			return std::min(len, skipBlockSize);
		}
	} else if(state == eTES_SendingVar) {
		String* val = &templateData[varName];
		if(varDataPos < val->length()) {
			debug_d("continue TRANSFER variable value (not completed)");
			available = val->length() - varDataPos;
			memcpy(data, ((char*)val->c_str()) + varDataPos, available);
			return available;
		} else {
			debug_d("continue to plaint text");
			state = eTES_Wait;
		}
	}

	int len = FileStream::readMemoryBlock(data, bufSize);
	char* tpl = data;
	if(tpl && len > 0) {
		char* end = tpl + len;
		char* cur = (char*)memchr(tpl, '{', len);
		char* lastFound = cur;
		while(cur != NULL) {
			lastFound = cur;
			char* p = cur + 1;
			for(; p < end; p++) {
				if(isspace(*p))
					break; // Not a var name
				else if(p - cur > TEMPLATE_MAX_VAR_NAME_LEN)
					break; // To long for var name
				else if(*p == '{')
					break; // New start..

				if(*p == '}') {
					int block = p - cur + 1;
					char varname[TEMPLATE_MAX_VAR_NAME_LEN + 1] = {0};
					memcpy(varname, cur + 1, p - cur - 1); // name without { and }
					varName = varname;
					state = eTES_Found;
					varWaitSize = cur - tpl;
					debug_d("found var: %s, at %d (%d) - %d, send size %d", varName.c_str(), varWaitSize + 1,
							varWaitSize + getPos(), p - tpl, varWaitSize);
					skipBlockSize = block;
					if(varWaitSize == 0)
						state = eTES_StartVar;
					return varWaitSize; // return only plain text from template without our variable
				}
			}
			cur = (char*)memchr(p, '{', len - (p - tpl)); // continue searching..
		}
		if(lastFound != NULL && (lastFound - tpl) > (len - TEMPLATE_MAX_VAR_NAME_LEN)) {
			debug_d("trim end to %d from %d", lastFound - tpl, len);
			len = lastFound - tpl; // It can be a incomplete variable name. Don't split it!
		}
	}

	debug_d("plain template text pos: %d, len: %d", getPos(), len);
	return len;
}

bool TemplateFileStream::seek(int len)
{
	if(len < 0)
		return false;
	//debug_d("SEEK: %d, (%d)", len, state);

	if(state == eTES_Found) {
		//debug_d("SEEK before Var: %d, (%d)", len, varWaitSize);
		varWaitSize -= len;
		if(varWaitSize == 0)
			state = eTES_StartVar;
	} else if(state == eTES_SendingVar) {
		varDataPos += len;
		return false; // not the end
	}

	return FileStream::seek(len);
}

void TemplateFileStream::setVar(String name, String value)
{
	templateData[name] = value;
}

void TemplateFileStream::setVars(const TemplateVariables& vars)
{
	templateData.setMultiple(vars);
}

JsonObjectStream::JsonObjectStream() : rootNode(buffer.createObject()), send(true)
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
	if(rootNode != JsonObject::invalid() && send) {
		int len = rootNode.printTo(*this);
		send = false;
	}

	return MemoryDataStream::readMemoryBlock(data, bufSize);
}

int JsonObjectStream::available()
{
	if(rootNode == JsonObject::invalid()) {
		return 0;
	}

	return rootNode.measureLength();
}

EndlessMemoryStream::~EndlessMemoryStream()
{
	delete stream;
	stream = NULL;
}

StreamType EndlessMemoryStream::getStreamType()
{
	return eSST_Memory;
}

uint16_t EndlessMemoryStream::readMemoryBlock(char* data, int bufSize)
{
	if(stream == NULL) {
		return 0;
	}

	return stream->readMemoryBlock(data, bufSize);
}

//Use base class documentation
bool EndlessMemoryStream::seek(int len)
{
	if(stream == NULL) {
		return false;
	}

	int res = stream->seek(len);
	if(stream->isFinished()) {
		delete stream;
		stream = NULL;
	}

	return res;
}

size_t EndlessMemoryStream::write(uint8_t charToWrite)
{
	if(stream == NULL) {
		stream = new MemoryDataStream();
	}

	return stream->write(charToWrite);
}

size_t EndlessMemoryStream::write(const uint8_t* buffer, size_t size)
{
	if(stream == NULL) {
		stream = new MemoryDataStream();
	}

	return stream->write(buffer, size);
}

bool EndlessMemoryStream::isFinished()
{
	return false;
}

LimitedMemoryStream::LimitedMemoryStream(size_t length)
{
	buffer = new uint8_t[length];
	this->length = length;
}

LimitedMemoryStream::~LimitedMemoryStream()
{
	delete[] buffer;
}

StreamType LimitedMemoryStream::getStreamType()
{
	return eSST_Memory;
}

uint16_t LimitedMemoryStream::readMemoryBlock(char* data, int bufSize)
{
	int written = std::min(bufSize, available());
	memcpy(data, buffer + readPos, written);

	return written;
}

bool LimitedMemoryStream::seek(int len)
{
	if(readPos + len > length) {
		return false;
	}

	readPos += len;

	return true;
}

size_t LimitedMemoryStream::write(uint8_t charToWrite)
{
	return write(&charToWrite, 1);
}

size_t LimitedMemoryStream::write(const uint8_t* data, size_t size)
{
	if(writePos + size <= length) {
		memcpy(buffer + writePos, data, size);
		writePos += size;
	}

	return size;
}

bool LimitedMemoryStream::isFinished()
{
	return (readPos >= length);
}
