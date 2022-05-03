
#include "../include/Hosted/Util.h"
#include <debug_progmem.h>

namespace Hosted
{

/**
 * Convert C type to format character
 * See: https://docs.python.org/3.5/library/struct.html#format-strings
 */
char convertType(const String& type){
	if(type == "int") {
		return 'i';
	}

	if(type == "uint8_t") {
		return 'B';
	}

	if(type == "uint16_t") {
		return 'H';
	}

	if(type == "bool") {
		return '?';
	}

	if(type == "char") {
		return 'c';
	}

	if(type == "float") {
		return 'f';
	}

	// TODO: ... add all types...
	if(type != "void") {
		debug_w("Uknknown type: %s", type);
	}

	// void and unknown
	return '\0';
}

/**
 * Converts a name as given from __PRETTY_FUNCTION__ to internal format.
 * Examples:
 * 			void a::sub(int) -> a::sub(: i)
 * 			void TwoWire::pins(uint8_t,uint8_t) -> TwoWire::pins(: B B)
 * 			uint8_t digitalRead(uint16_t) -> digitalRead(B: H)
 * @param name source name
 * @retval converted name
 */
String convertFQN(const String& name)
{
	String converted;
	int spacePos = name.indexOf(" ");
	String returnType = name.substring(0, name.indexOf(" "));
	int openBracePosition = name.indexOf("(");
	int closeBracePosition = name.indexOf(")");
	Vector<String> params;
	String arguments = name.substring(openBracePosition + 1, closeBracePosition);
	int paramsCount = splitString(arguments, ',', params);

	converted = name.substring(spacePos + 1, openBracePosition) + "("; // name
	char returnTypeChar = convertType(returnType);
	if(returnTypeChar) {
		converted  += returnTypeChar;
	}

	converted += ":";
	for(size_t i=0; i< params.count(); i++) {
		params[i].trim();
		converted += " " + String(convertType(params[i]));
	}
	converted += ")";

	return converted;
}

} // namespace
