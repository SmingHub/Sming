/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StreamChain.h
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "MultiStream.h"
#include "../ObjectQueue.h"

#ifndef MAX_STREAM_CHAIN_SIZE
/**
 * @brief Limit on number of streams in a chain
 */
#define MAX_STREAM_CHAIN_SIZE 10
#endif

/**
 * @brief Provides a read-only stream which concatenates content from multiple source streams
 *
 * Attached streams are released as soon as they are read out.
 */
class StreamChain : public MultiStream
{
public:
	~StreamChain()
	{
		// Free any remaining streams in queue
		while(queue.count() != 0) {
			delete queue.dequeue();
		}
	}

	bool attachStream(IDataSourceStream* stream)
	{
		return queue.enqueue(stream);
	}

protected:
	IDataSourceStream* getNextStream() override
	{
		return queue.dequeue();
	}

private:
	using Queue = ObjectQueue<IDataSourceStream, MAX_STREAM_CHAIN_SIZE>;

	Queue queue;
};
