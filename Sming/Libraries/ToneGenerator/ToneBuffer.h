/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ToneBuffer.h - Support for simple tone generation via I2S
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <cstdint>

class ToneBufferQueue;

/**
 * @brief Contains samples for one full signal cycle at a specific frequency
 * @note Data is stored Delta-Sigma modulated to minimise I2S transfer overhead.
 */
class ToneBuffer
{
public:
	typedef uint32_t Sample;
	static constexpr auto sampleSize = sizeof(Sample);

	~ToneBuffer()
	{
		delete next;
		delete data;
	}

	bool allocate(unsigned sampleCount)
	{
		if(capacity < sampleCount) {
			delete data;
			data = new Sample[sampleCount];
			if(data == nullptr) {
				this->sampleCount = 0;
				return false;
			}
		}

		this->sampleCount = sampleCount;
		return true;
	}

	Sample* getData()
	{
		return data;
	}

	unsigned getCapacity()
	{
		return capacity;
	}

	unsigned getSampleCount()
	{
		return sampleCount;
	}

	unsigned repeatCount = 0;

private:
	friend ToneBufferQueue;
	ToneBuffer* next = nullptr;
	Sample* data = nullptr;
	unsigned capacity = 0;
	unsigned sampleCount = 0;
};
