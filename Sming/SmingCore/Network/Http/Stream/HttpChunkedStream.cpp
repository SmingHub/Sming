#include "HttpChunkedStream.h"

HttpChunkedStream::HttpChunkedStream(ReadWriteStream *stream)
{
	this->stream = stream;
}

HttpChunkedStream::~HttpChunkedStream()
{
	delete tempStream;
	delete stream;
	tempStream = NULL;
	stream = NULL;
}

size_t HttpChunkedStream::write(uint8_t charToWrite)
{
	return stream->write(charToWrite);
}

size_t HttpChunkedStream::write(const uint8_t *buffer, size_t size)
{
	return stream->write(buffer, size);
}

uint16_t HttpChunkedStream::readMemoryBlock(char* data, int bufSize)
{
	const int readSize = NETWORK_SEND_BUFFER_SIZE;

	if(stream == NULL || stream->isFinished()) {
		return 0;
	}

	if(tempStream == NULL) {
		tempStream = new CircularBuffer(readSize + 10);
	}

	if(!tempStream->isFinished()) {
		return tempStream->readMemoryBlock(data, bufSize);
	}

	// pump new data into the stream
	int len = readSize;
	char buffer[len];
	len = stream->readMemoryBlock(buffer, len);
	stream->seek(max(len, 0));
	if(len < 1) {
		return 0;
	}

	String content = String(len) + "\r\n";
	tempStream->write((uint8_t*)content.c_str(), content.length());
	tempStream->write((uint8_t*)buffer, len);
	content = "\n\r";
	tempStream->write((uint8_t*)content.c_str(), content.length());
	if (len < readSize) {
		content = "0\r\n\r\n";
		tempStream->write((uint8_t*)content.c_str(), content.length());
	}

	return tempStream->readMemoryBlock(data, bufSize);
}

//Use base class documentation
bool HttpChunkedStream::seek(int len)
{
	return tempStream->seek(len);
}

//Use base class documentation
bool HttpChunkedStream::isFinished()
{
	return (stream->isFinished() && tempStream->isFinished());
}
