/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_HTTP_MPARTSTREAM_H_
#define _SMING_CORE_HTTP_MPARTSTREAM_H_

#include "../HttpCommon.h"
#include "../HttpResponse.h"
#include "../HttpRequest.h"
#include "../../../Delegate.h"

/**
 * @brief      HTTP multipart stream class
 * @ingroup    stream http
 *
 *  @{
*/

typedef struct {
	HttpHeaders* headers = NULL;
	ReadWriteStream* stream  = NULL;
} HttpPartResult;

typedef Delegate <HttpPartResult()> HttpPartProducerDelegate;

class HttpMultipartStream: public ReadWriteStream
{
public:
	HttpMultipartStream(HttpPartProducerDelegate delegate);
	virtual ~HttpMultipartStream();

	//Use base class documentation
	virtual StreamType getStreamType() {
		// TODO: fix this...
		return stream->getStreamType();
	}

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	*/
	int length() { return -1; }

	/** @brief  Write a single char to stream
	 *  @param  charToWrite Char to write to the stream
	 *  @retval size_t Quantity of chars written to stream (always 1)
	 */
	virtual size_t write(uint8_t charToWrite);

	/** @brief  Write chars to stream
	 *  @param  buffer Pointer to buffer to write to the stream
	 *  @param  size Quantity of chars to written
	 *  @retval size_t Quantity of chars written to stream
	 */
	virtual size_t write(const uint8_t *buffer, size_t size);

	//Use base class documentation
	virtual uint16_t readMemoryBlock(char* data, int bufSize);

	//Use base class documentation
	virtual bool seek(int len);

	//Use base class documentation
	virtual bool isFinished();

	/**
	 * @brief Returns the generated boundary
	 *
	 * @retval const char*
	 */
	const char* getBoundary();

private:
	HttpPartProducerDelegate producer;

	ReadWriteStream *stream = NULL;
	ReadWriteStream *nextStream = NULL;

	char boundary[16] = {0};
};

/** @} */
#endif /* _SMING_CORE_HTTP_MPARTSTREAM_H_ */
