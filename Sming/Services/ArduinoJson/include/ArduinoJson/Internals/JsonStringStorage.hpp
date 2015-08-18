#ifndef _JSON_STRING_STORAGE_
#define _JSON_STRING_STORAGE_

#include "../../../../../Wiring/WString.h"
#include "JsonBufferAllocated.hpp"
#include "ReferenceType.hpp"

namespace ArduinoJson {

// Forward declarations
class JsonArray;
class JsonBuffer;

class JsonStringStorage : public Internals::ReferenceType,
                  	  	  public Internals::JsonBufferAllocated
{
public:
	JsonStringStorage(const String text)
	{
		stor = text;
	}
	String& value() { return stor; }
	const char* c_str() { return stor.c_str(); }

	// Returns a reference an invalid JsonStringStorage.
	// This object is meant to replace a NULL pointer.
	// This is used when memory allocation or JSON parsing fail.
	static JsonStringStorage &invalid() { return _invalid; }

private:
	String stor;

	// The instance returned by JsonArray::invalid()
	static JsonStringStorage _invalid;
};

}

#endif
