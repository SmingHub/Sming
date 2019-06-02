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
#define MAX_STREAM_CHAIN_SIZE 10
#endif

typedef ObjectQueue<IDataSourceStream, MAX_STREAM_CHAIN_SIZE> StreamChainQueue;

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
	StreamChainQueue queue;
};
