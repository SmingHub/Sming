#include "HttpCommon.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "../TcpClient.h"
#include "../../DataSourceStream.h"


class HttpChunkedStream: public ReadWriteStream
{
public:
	HttpChunkedStream(ReadWriteStream *stream);
	virtual ~HttpChunkedStream();

	//Use base class documentation
	virtual StreamType getStreamType() { return stream->getStreamType(); }

	/**
	 * @brief Return the total length of the stream
	 * @retval int -1 is returned when the size cannot be determined
	*/
	int length() { return stream->length(); }

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

private:
	ReadWriteStream *stream = NULL;
	MemoryDataStream *tempStream = NULL;
};
