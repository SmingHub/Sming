/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "JsonObjectStream.h"

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
		rootNode.printTo(*this);
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
