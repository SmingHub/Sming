/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_DATA_READ_STREAM_CHAIN_H_
#define _SMING_CORE_DATA_READ_STREAM_CHAIN_H_

#include "MultiStream.h"
#include "../ObjectQueue.h"

#ifndef MAX_STREAM_CHAIN_SIZE
#define MAX_STREAM_CHAIN_SIZE 10
#endif

typedef ObjectQueue<ReadWriteStream, MAX_STREAM_CHAIN_SIZE> StreamChainQueue;

class StreamChain : public MultiStream
{
public:
	virtual ~StreamChain()
	{
	}

	bool attachStream(ReadWriteStream* stream)
	{
		return queue.enqueue(stream);
	}

protected:
	virtual ReadWriteStream* getNextStream()
	{
		return queue.dequeue();
	}

private:
	StreamChainQueue queue;
};

#endif /* _SMING_CORE_DATA_READ_STREAM_CHAIN_H_ */
