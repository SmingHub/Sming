/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "StreamTransformer.h"

#define NETWORK_SEND_BUFFER_SIZE 1024

StreamTransformer::StreamTransformer(IDataSourceStream* stream, const StreamTransformerCallback& callback,
									 size_t resultSize /* = 256 */, size_t blockSize /* = 64 */
									 )
	: transformCallback(callback)
{
	sourceStream = stream;
	this->resultSize = resultSize;
	result = new uint8_t[this->resultSize];
	this->blockSize = blockSize;
}

StreamTransformer::~StreamTransformer()
{
	delete[] result;
	delete tempStream;
	delete sourceStream;
	result = nullptr;
	tempStream = nullptr;
	sourceStream = nullptr;
}

uint16_t StreamTransformer::readMemoryBlock(char* data, int bufSize)
{
	if(tempStream == nullptr) {
		tempStream = new CircularBuffer(NETWORK_SEND_BUFFER_SIZE + 10);
	}

	if(tempStream->isFinished()) {
		if(sourceStream->isFinished()) {
			return 0;
		}

		// Fill the temp stream with data...
		int i = bufSize / blockSize + 1;
		do {
			int len = blockSize;
			if(i == 1) {
				len = bufSize % blockSize;
			}

			len = sourceStream->readMemoryBlock(data, len);
			if(!len) {
				break;
			}

			saveState();
			int outLength = transformCallback((uint8_t*)data, len, result, resultSize);
			if(outLength > tempStream->room()) {
				restoreState();
				break;
			}

			if(tempStream->write(result, outLength) != outLength) {
				debug_e("That should not happen!");
				restoreState();
				break;
			}

			sourceStream->seek(len);
		} while(--i);

		if(sourceStream->isFinished()) {
			int outLength = transformCallback(nullptr, 0, result, resultSize);
			tempStream->write(result, outLength);
		}

	} /* if(tempStream->isFinished()) */

	return tempStream->readMemoryBlock(data, bufSize);
}

//Use base class documentation
bool StreamTransformer::seek(int len)
{
	return tempStream->seek(len);
}

//Use base class documentation
bool StreamTransformer::isFinished()
{
	return (sourceStream->isFinished() && tempStream->isFinished());
}
