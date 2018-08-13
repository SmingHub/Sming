
#include "ChunkedStream.h"

/**
 * Encodes a chunk of data
 * @param uint8_t* source - the incoming data
 * @param size_t sourceLength -length of the incoming data
 * @param uint8_t* target - the result data. The pointer must point to an already allocated memory
 * @param int* targetLength - the length of the result data
 *
 * @return the length of the encoded target.
 */
static size_t chunkEncode(const uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength)
{
	char head[16];
	int headLen = m_snprintf(head, sizeof(head), _F("%X\r\n"), sourceLength);

	const char* tail = "\r\n";
	int tailLen = 2;

	if (headLen + sourceLength + tailLen > targetLength)
		return 0;

	auto p = target;
	memcpy(p, head, headLen);
	p += headLen;
	if (sourceLength) {
		memcpy(p, source, sourceLength);
		p += sourceLength;
	}
	memcpy(p, tail, tailLen);
	p += tailLen;

	return p - target;
}

ChunkedStream::ChunkedStream(IDataSourceStream* stream, size_t resultSize)
	: StreamTransformer(stream, chunkEncode, resultSize, resultSize - 12)
{}
