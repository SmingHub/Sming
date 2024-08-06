/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WriteBuffer.h
 *
 ****/

#pragma once

#include <Data/Stream/ReadWriteStream.h>
#include <memory>

/**
 * @brief Generic write-through buffer class
 * @ingroup stream
 * @note Call flush() at end of write operation to ensure all data is output
 * This is done automatically when the buffer is destroyed.
 */
class BaseWriteBuffer : public Print
{
public:
	/**
     * @brief Create buffer
     * @param output Destination stream
     * @param buffer buffer to use
     * @param size Size of buffer
     */
	BaseWriteBuffer(Print& output, uint8_t buffer[], size_t bufferSize)
		: output(output), buffer(buffer), bufferSize(bufferSize)
	{
	}

	~BaseWriteBuffer()
	{
		flush();
	}

	size_t write(uint8_t c) override;

	size_t write(const uint8_t* data, size_t size) override;

	/**
     * @brief Write any buffered content to output
     */
	void flush();

private:
	Print& output;
	uint8_t* buffer;
	size_t bufferSize;
	size_t writeOffset{};
};

/**
 * @brief Write-through buffer using stack only
 * @tparam size Size of buffer
 *
 * Example usage:
 *
 *      FileStream stream("file.txt", File::ReadWrite);
 * 	    {
 *	        StaticWriteBuffer<256> buffer(stream);
 * 	        writeSomeData(buffer);
 * 	    } // Buffer flushed and destroyed when it goes out of scope
 */
template <size_t size> class StaticWriteBuffer : public BaseWriteBuffer
{
public:
	/**
     * @brief Construct a stack-based buffer
     * @param output Print destination
     */
	StaticWriteBuffer(Print& output) : BaseWriteBuffer(output, buffer, size)
	{
	}

private:
	uint8_t buffer[size];
};

/**
 * @brief Write-through buffer using heap storage
 *
 * Example usage:
 *
 *      FileStream stream("file.txt", File::ReadWrite);
 * 	    {
 *	        HeapWriteBuffer buffer(stream, 512);
 * 	        writeSomeData(buffer);
 * 	    } // Buffer flushed and destroyed when it goes out of scope
 */
class HeapWriteBuffer : public BaseWriteBuffer
{
public:
	/**
     * @brief Construct a stack-based buffer
     * @param output Print destination
     * @param size Buffer size
     */
	HeapWriteBuffer(Print& output, size_t size) : HeapWriteBuffer(output, new uint8_t[size], size)
	{
	}

private:
	HeapWriteBuffer(Print& output, uint8_t* buffer, size_t size) : BaseWriteBuffer(output, buffer, size), buffer(buffer)
	{
	}

	std::unique_ptr<uint8_t[]> buffer;
};

/**
 * @brief Write-through buffer using heap storage and owned stream pointer
 *
 * Example usage:
 *
 *      auto stream = std::make_unique<FileStream>("file.txt", File::ReadWrite);
 *	    auto bufferedStream = new DynamicWriteBuffer(std::move(stream), 512);
 *
 *      // write to bufferedStream as required via callbacks, etc.
 *      ...
 *
 *      // This destroys both buffer *and* the file stream
 *      delete bufferedStream;
 */
class DynamicWriteBuffer : public BaseWriteBuffer
{
public:
	/**
     * @brief Construct a stack-based buffer
     * @param output Print destination, will take ownership of this
     * @param size Buffer size
     */
	DynamicWriteBuffer(std::unique_ptr<Print>&& output, size_t size)
		: DynamicWriteBuffer(output.release(), new uint8_t[size], size)
	{
	}

	~DynamicWriteBuffer()
	{
		flush();
	}

private:
	DynamicWriteBuffer(Print* output, uint8_t* buffer, size_t size)
		: BaseWriteBuffer(*output, buffer, size), output(output), buffer(buffer)
	{
	}

	std::unique_ptr<Print> output;
	std::unique_ptr<uint8_t[]> buffer;
};
