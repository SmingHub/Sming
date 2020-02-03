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
 * @brief Read-only stream for creating HTTP multi-part content
 * @see See https://www.w3.org/Protocols/rfc1341/7_2_Multipart.html
 * @ingroup stream data
*/
class MultipartStream : public MultiStream
{
public:
	/**
	 * @brief Each result item contains a set of headers plus content stream
	 */
	struct BodyPart {
		HttpHeaders* headers = nullptr;
		IDataSourceStream* stream = nullptr;
	};

	/**
	 * @brief Callback used to produce each result
	 */
	using Producer = Delegate<BodyPart()>;

	MultipartStream(Producer delegate) : producer(delegate)
	{
	}

	~MultipartStream()
	{
		delete bodyPart.headers;
		delete bodyPart.stream;
	}

	/**
	 * @brief Returns the generated boundary
	 *
	 * @retval const char*
	 */
	const char* getBoundary();

protected:
	IDataSourceStream* getNextStream() override;

private:
	Producer producer;
	BodyPart bodyPart;
	char boundary[16] = {0};
};

/**
 * @deprecated Use `MultipartStream::BodyPart` instead
 */
typedef MultipartStream::BodyPart HttpPartResult SMING_DEPRECATED;

/**
 * @deprecated Use `MultipartStream::Producer` instead
 */
typedef MultipartStream::Producer HttpPartProducerDelegate SMING_DEPRECATED;
