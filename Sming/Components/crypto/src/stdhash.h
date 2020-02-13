/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * hash.h - Code templates for standard hash functions
 *
 ****/

#pragma once

#include "util.h"
#include <cstring>
#include <sys/pgmspace.h>

using namespace Crypto::Internal;

namespace Crypto
{
namespace Internal
{
/**
 * @brief Block hash operation transposes incoming block of data into state
 * @param state Intermediate digest state, an array of uint32_t or uint64_t
 * @param block Complete block of data to be processed
 */
template <typename State> using HashProcess = void(State state[], const uint8_t block[]);

/**
 * @brief Perform a hash update with some message data
 * @param ctx Hash function context containing state, count and buffer elements
 * @param process Called to process a complete block of data
 * @param input The input message
 * @param length Number of bytes in the message
 */
template <typename Context, typename T = decltype(Context::state[0])>
void hashUpdate(Context* ctx, HashProcess<T> process, const void* input, size_t length)
{
	auto msg = static_cast<const uint8_t*>(input);
	auto bufsize = sizeof(ctx->buffer);
	auto bufpos = uint32_t(ctx->count) % bufsize;

	ctx->count += length;

	// If there's any buffered data, append and process if possible
	auto bufspace = bufsize - bufpos;
	if(bufspace > 0 && length >= bufspace) {
		memcpy(&ctx->buffer[bufpos], msg, bufspace);
		process(ctx->state, ctx->buffer);
		length -= bufspace;
		msg += bufspace;
		bufpos = 0;
	}

	// Process complete blocks directly from input, buffer not required
	while(length >= bufsize) {
		process(ctx->state, msg);
		length -= bufsize;
		msg += bufsize;
	}

	// Buffer any remaining bytes
	if(length != 0) {
		memcpy(&ctx->buffer[bufpos], msg, length);
	}
}

/**
 * @brief Perform final padding and bitsize encoding
 * @tparam byteswap true for algorithms which store count MSB first
 * @param ctx
 * @param digest Buffer for final digest
 */
template <bool byteSwap, class Context, typename T = decltype(Context::state[0])>
void hashFinal(const Context* ctx, HashProcess<T> process, decltype(Context::state)& digest)
{
	// Use a temporary block buffer so context remains intact
	decltype(ctx->buffer) buf;
	auto bufsize = sizeof(buf);
	auto bufpos = ctx->count % sizeof(buf);
	memcpy(buf, ctx->buffer, bufpos);

	// Copy the intermediate state
	memcpy(digest, ctx->state, sizeof(digest));

	buf[bufpos++] = 0x80;
	auto countPos = bufsize - (2 * sizeof(T));
	if(bufpos > countPos) {
		memset(buf + bufpos, 0, bufsize - bufpos);
		process(digest, buf);
		memset(buf, 0, countPos);
	} else {
		memset(buf + bufpos, 0, countPos - bufpos);
	}

	auto bitCount = uint64_t(ctx->count) * 8;
	if(byteSwap) {
		if(sizeof(T) == sizeof(uint64_t)) {
			encbe(buf + countPos, T(0));
			encbe(buf + countPos + sizeof(T), bitCount);
		} else {
			encbe(buf + countPos, bitCount);
		}
	} else {
		memcpy(buf + countPos, &bitCount, sizeof(bitCount));
	}

	process(digest, buf);
}

} // namespace Internal
} // namespace Crypto
