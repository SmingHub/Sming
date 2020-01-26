/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * blake2s.cpp - Blake2s implementation, partially unrolled
 *
 ****/

#include "../include/Crypto/Blake2s.h"
#include "util.h"
#include <FakePgmSpace.h>

namespace Crypto
{
namespace
{
static const uint32_t initVectors[8] PROGMEM = {0x6A09E667U, 0xBB67AE85U, 0x3C6EF372U, 0xA54FF53AU,
												0x510E527FU, 0x9B05688CU, 0x1F83D9ABU, 0x5BE0CD19U};

template <typename Context> class Compressor
{
public:
	explicit Compressor(Context& ctx) : context(ctx)
	{
	}

	void operator()(const uint8_t* block, uint32_t increment, bool isFinal = false)
	{
		context.count += increment;
		memcpy(m, block, sizeof(m));
		std::copy_n(context.state, 8, v);
		v[8] = initVectors[0];
		v[9] = initVectors[1];
		v[10] = initVectors[2];
		v[11] = initVectors[3];
		v[12] = initVectors[4] ^ uint32_t(context.count);
		v[13] = initVectors[5] ^ uint32_t(context.count >> 32);
		v[14] = initVectors[6] ^ (isFinal ? 0xFFFFFFFFU : 0U);
		v[15] = initVectors[7];

		// Full loop unrolling would increase code size by approx. 3kB, but only reduce
		// runtime by about 30%.
		shuffleRound<0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>();
		shuffleRound<14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3>();
		shuffleRound<11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4>();
		shuffleRound<7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8>();
		shuffleRound<9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13>();
		shuffleRound<2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9>();
		shuffleRound<12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11>();
		shuffleRound<13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10>();
		shuffleRound<6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5>();
		shuffleRound<10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0>();

		for(auto i = 0; i < 8; ++i) {
			context.state[i] ^= v[i] ^ v[i + 8];
		}
	}

private:
	template <size_t M0, size_t M1, size_t M2, size_t M3, size_t M4, size_t M5, size_t M6, size_t M7, size_t M8,
			  size_t M9, size_t M10, size_t M11, size_t M12, size_t M13, size_t M14, size_t M15>
	void shuffleRound()
	{
		mPerm[0] = m[M0];
		mPerm[1] = m[M1];
		mPerm[2] = m[M2];
		mPerm[3] = m[M3];
		mPerm[4] = m[M4];
		mPerm[5] = m[M5];
		mPerm[6] = m[M6];
		mPerm[7] = m[M7];
		mPerm[8] = m[M8];
		mPerm[9] = m[M9];
		mPerm[10] = m[M10];
		mPerm[11] = m[M11];
		mPerm[12] = m[M12];
		mPerm[13] = m[M13];
		mPerm[14] = m[M14];
		mPerm[15] = m[M15];
		round();
	}

	void round()
	{
		roundPart<0>(v[0], v[4], v[8], v[12]);
		roundPart<1>(v[1], v[5], v[9], v[13]);
		roundPart<2>(v[2], v[6], v[10], v[14]);
		roundPart<3>(v[3], v[7], v[11], v[15]);
		roundPart<4>(v[0], v[5], v[10], v[15]);
		roundPart<5>(v[1], v[6], v[11], v[12]);
		roundPart<6>(v[2], v[7], v[8], v[13]);
		roundPart<7>(v[3], v[4], v[9], v[14]);
	}

	template <size_t I> void roundPart(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d)
	{
		using namespace Crypto::Internal;

		a += b + mPerm[2 * I];
		d = ROTR(d ^ a, 16);
		c += d;
		b = ROTR(b ^ c, 12);
		a += b + mPerm[2 * I + 1];
		d = ROTR(d ^ a, 8);
		c += d;
		b = ROTR(b ^ c, 7);
	}

	uint32_t mPerm[16];
	uint32_t v[16];
	uint32_t m[16];
	Context& context;
};

} // namespace

void Blake2sEngine::initCommon(size_t hashSize, size_t keySize)
{
	assert(hashSize > 0 && hashSize <= hashsize);
	assert(keySize <= maxkeysize);

	context = {};
	userHashSize = hashSize;

	std::copy_n(initVectors, 8, context.state);
	context.state[0] ^= (0x01010000U | (keySize << 8) | hashSize);
}

void Blake2sEngine::init(const Secret& key, size_t hashSize)
{
	initCommon(hashSize, key.size());

	uint8_t block[blocksize] = {0};
	memcpy(block, key.data(), key.size());
	update(block, blocksize);
	Internal::clean(block);
}

void Blake2sEngine::update(const void* data, size_t size)
{
	if(size == 0) {
		return;
	}

	const uint8_t* pData = reinterpret_cast<const uint8_t*>(data);
	Compressor<Context> compressor(context);

	const size_t bufferSpace = blocksize - context.bufferLength;
	if(size > bufferSpace) {
		memcpy(context.buffer + context.bufferLength, pData, bufferSpace);
		compressor(context.buffer, blocksize);
		context.bufferLength = 0;
		pData += bufferSpace;
		size -= bufferSpace;
	}

	// exclude last block if size is a multiple of the block size
	while(size > blocksize) {
		compressor(pData, blocksize);
		pData += blocksize;
		size -= blocksize;
	}

	if(size > 0) {
		assert(context.bufferLength + size <= blocksize);
		memcpy(context.buffer + context.bufferLength, pData, size);
		context.bufferLength += size;
	}
}

void Blake2sEngine::final(uint8_t* hash)
{
	// setup padding for last block
	std::fill_n(context.buffer + context.bufferLength, blocksize - context.bufferLength, 0);
	Compressor<Context> compressor(context);
	compressor(context.buffer, context.bufferLength, 0xFFFFFFFF);

	memcpy(hash, context.state, userHashSize);
	Internal::clean(context);
}

} // namespace Crypto
