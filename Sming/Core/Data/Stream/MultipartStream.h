/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MultipartStream.h
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#pragma once

#include "MultiStream.h"
#include "Network/Http/HttpHeaders.h"

/**
 * @brief      Multipart stream class
 * @ingroup    stream data
 *
 *  @{
*/

typedef struct {
	HttpHeaders* headers = nullptr;
	IDataSourceStream* stream = nullptr;
} HttpPartResult;

typedef Delegate<HttpPartResult()> HttpPartProducerDelegate;

class MultipartStream : public MultiStream
{
public:
	MultipartStream(HttpPartProducerDelegate delegate) : producer(delegate)
	{
	}

	/**
	 * @brief Returns the generated boundary
	 *
	 * @retval const char*
	 */
	const char* getBoundary();

protected:
	IDataSourceStream* getNextStream() override
	{
		result = producer();
		return result.stream;
	}

	void onNextStream() override;
	bool onCompleted() override;

private:
	HttpPartProducerDelegate producer;
	HttpPartResult result;

	char boundary[16] = {0};
};

/** @} */
