/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RtttlJsonListStream.h - Support for streaming tune files in JSON format
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "RtttlParser.h"
#include <Data/Stream/DataSourceStream.h>

/**
 * @brief A forward-only stream for listing contents of a tune file
 * @note Tune files can be large so we only output one tune title at a time
 */
class RtttlJsonListStream : public IDataSourceStream
{
public:
	/**
	 * @brief Construct a list stream
	 * @param name Identifies this stream, will have .json appended
	 * @param parser Pre-initialised parser to obtain tunes from
	 */
	RtttlJsonListStream(const String& name, RingTone::RtttlParser* parser) : name(name), parser(parser)
	{
		begin();
	}

	~RtttlJsonListStream()
	{
		delete parser;
	}

	bool isValid() const override
	{
		return parser != nullptr;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	bool seek(int len) override;

	bool isFinished() override
	{
		return state >= 3;
	}

	String getName() const override;

	unsigned getIndex()
	{
		return index;
	}

protected:
	void begin();

private:
	String name;
	String title;
	RingTone::RtttlParser* parser;
	uint8_t state = 0;
	unsigned index = 0;
	unsigned readPos = 0;
};
