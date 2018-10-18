/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "TemplateFileStream.h"

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
