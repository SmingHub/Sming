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

#define TEMP_BUFFER_SIZE (1024 + 10)


StreamTransformer::StreamTransformer(IDataSourceStream* stream, const StreamTransformerCallback& callback,
										size_t resultSize, size_t blockSize) :
	_transformCallback(callback),
	_sourceStream(stream),
	_resultSize(resultSize),
	_blockSize(blockSize)
{
	_result = new uint8_t[_resultSize];
}


StreamTransformer::~StreamTransformer()
{
	delete[] _result;
	delete _tempStream;
	delete _sourceStream;
}


size_t StreamTransformer::readMemoryBlock(char* data, size_t bufSize)
{
	if (!_tempStream)
		_tempStream = new CircularBuffer(TEMP_BUFFER_SIZE);

	if (_tempStream->isFinished()) {

		if (_sourceStream->isFinished())
			return 0;

		// Fill the temp stream with data...
		unsigned i = (bufSize + _blockSize - 1) / _blockSize;
		while (i--) {
			size_t len = _blockSize;
			if (i == 0)
				len = bufSize % _blockSize;

			len = _sourceStream->readMemoryBlock(data, len);
			if (len == 0)
				break;

			saveState();
			size_t outLength = _transformCallback((uint8_t*)data, len, _result, _resultSize);
			if (outLength > _tempStream->room()) {
				restoreState();
				break;
			}

			if ( _tempStream->write(_result, outLength) != outLength) {
				debug_e("That should not happen!");
				restoreState();
				break;
			}

			_sourceStream->seek(len);
		}

		if (_sourceStream->isFinished()) {
			int outLength = _transformCallback(nullptr, 0, _result, _resultSize);
			_tempStream->write(_result, outLength);
		}

	} /* if (tempStream->isFinished()) */

	return _tempStream->readMemoryBlock(data, bufSize);
}


bool StreamTransformer::seek(int len)
{
	return _tempStream->seek(len);
}


bool StreamTransformer::isFinished()
{
	return (_sourceStream->isFinished() && _tempStream->isFinished());
}
