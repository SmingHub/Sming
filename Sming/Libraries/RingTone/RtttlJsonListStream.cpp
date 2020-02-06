/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RtttlJsonListStream.cpp
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "include/RtttlJsonListStream.h"

DEFINE_FSTR_LOCAL(jsonOpen, "{\"list\":[");
DEFINE_FSTR_LOCAL(jsonClose, "]}");

void RtttlJsonListStream::begin()
{
	title = '"';
	title += parser->getTitle();
	title += '"';
}

uint16_t RtttlJsonListStream::readMemoryBlock(char* data, int bufSize)
{
	if(bufSize <= 0) {
		return 0;
	}

	switch(state) {
	case 0:
		return jsonOpen.read(readPos, data, bufSize);

	case 1: {
		auto len = std::min(size_t(bufSize), title.length() - readPos);
		memcpy(data, &title[readPos], len);
		return len;
	}

	case 2:
		return jsonClose.read(readPos, data, bufSize);

	default:
		return 0;
	}
}

bool RtttlJsonListStream::seek(int len)
{
	if(len < 0) {
		return false;
	}

	auto skip = [&](size_t elemSize) -> bool {
		auto newPos = readPos + len;
		if(newPos > elemSize) {
			debug_e("seek(%d) out of range, max %u", len, elemSize);
			return false;
		}
		if(newPos == elemSize) {
			readPos = 0;
		} else {
			readPos = newPos;
		}
		return true;
	};

	switch(state) {
	case 0:
		if(!skip(jsonOpen.length())) {
			return false;
		}
		if(readPos == 0) {
			++state;
		}
		return true;

	case 1: {
		if(!skip(title.length())) {
			return false;
		}
		if(readPos == 0) {
			++index;
			if(parser->nextTune()) {
				title = ",\"";
				title += parser->getTitle();
				title += '"';
			} else {
				title = nullptr;
				++state;
			}
		}
		return true;
	}

	case 2:
		if(!skip(jsonClose.length())) {
			return false;
		}
		++state;
		return true;

	default:
		return false;
	}
}

String RtttlJsonListStream::getName() const
{
	return name + _F(".json");
}
