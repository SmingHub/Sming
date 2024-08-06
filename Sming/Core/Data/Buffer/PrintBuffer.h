/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * PrintBuffer.h
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
class BasePrintBuffer : public Print
{
public:
	/**
     * @brief Create buffer
     * @param output Destination stream
     * @param buffer buffer to use
     * @param size Size of buffer
     */
	BasePrintBuffer(Print& output, uint8_t buffer[], size_t bufferSize)
		: output(output), buffer(buffer), bufferSize(bufferSize)
	{
	}

	~BasePrintBuffer()
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
 *	        StaticPrintBuffer<256> buffer(stream);
 * 	        writeSomeData(buffer);
 * 	    } // Buffer flushed and destroyed when it goes out of scope
 */
template <size_t size> class StaticPrintBuffer : public BasePrintBuffer
{
public:
	/**
     * @brief Construct a stack-based buffer
     * @param output Print destination
     */
	StaticPrintBuffer(Print& output) : BasePrintBuffer(output, buffer, size)
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
 *	        HeapPrintBuffer buffer(stream, 512);
 * 	        writeSomeData(buffer);
 * 	    } // Buffer flushed and destroyed when it goes out of scope
 */
class HeapPrintBuffer : public BasePrintBuffer
{
public:
	/**
     * @brief Construct a stack-based buffer
     * @param output Print destination
     * @param size Buffer size
     */
	HeapPrintBuffer(Print& output, size_t size) : HeapPrintBuffer(output, new uint8_t[size], size)
	{
	}

private:
	HeapPrintBuffer(Print& output, uint8_t* buffer, size_t size) : BasePrintBuffer(output, buffer, size), buffer(buffer)
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
 *	    auto bufferedStream = new DynamicPrintBuffer(std::move(stream), 512);
 *
 *      // write to bufferedStream as required via callbacks, etc.
 *      ...
 *
 *      // This destroys both buffer *and* the file stream
 *      delete bufferedStream;
 */
class DynamicPrintBuffer : public BasePrintBuffer
{
public:
	/**
     * @brief Construct a stack-based buffer
     * @param output Print destination, will take ownership of this
     * @param size Buffer size
     */
	DynamicPrintBuffer(std::unique_ptr<Print>&& output, size_t size)
		: DynamicPrintBuffer(output.release(), new uint8_t[size], size)
	{
	}

	~DynamicPrintBuffer()
	{
		flush();
	}

private:
	DynamicPrintBuffer(Print* output, uint8_t* buffer, size_t size)
		: BasePrintBuffer(*output, buffer, size), output(output), buffer(buffer)
	{
	}

	std::unique_ptr<Print> output;
	std::unique_ptr<uint8_t[]> buffer;
};
