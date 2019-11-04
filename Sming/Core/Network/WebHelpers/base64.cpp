/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "base64.h"

#include "libb64/cencode.h"
#include "libb64/cdecode.h"

// Base-64 encoding produces 3 output bytes for every 2 input bytes
#define MIN_ENCODE_LEN(_in_len) (((_in_len)*3 + 1) / 2)
#define MIN_DECODE_LEN(_in_len) (((_in_len)*2 + 2) / 3)

int base64_encode(size_t in_len, const unsigned char* in, size_t out_len, char* out)
{
	// Base-64 encoding produces 3 output bytes for every 2 input bytes
	unsigned min_out_len = MIN_ENCODE_LEN(in_len);
	if(out_len < min_out_len)
		return -1;

	base64_encodestate state;
	base64_init_encodestate(&state, 0); // Don't include any linebreaks
	int codelength = base64_encode_block((const char*)in, in_len, out, &state);
	codelength += base64_encode_blockend(out + codelength, &state);
	return codelength;
}

String base64_encode(const unsigned char* in, size_t in_len)
{
	String s;
	if(!s.setLength(MIN_ENCODE_LEN(in_len)))
		return nullptr;

	int len = base64_encode(in_len, in, s.length(), s.begin());
	if(len < 0)
		return nullptr;

	s.setLength(len);
	return s;
}

/* decode a base64 string in one shot */
int base64_decode(size_t in_len, const char* in, size_t out_len, unsigned char* out)
{
	if(out_len < MIN_DECODE_LEN(in_len))
		return -1;

	base64_decodestate _state;
	base64_init_decodestate(&_state);
	return base64_decode_block(in, in_len, (char*)out, &_state);
}

String base64_decode(const char* in, size_t in_len)
{
	String s;
	if(!s.setLength(MIN_DECODE_LEN(in_len)))
		return nullptr;

	int outlen = base64_decode(in_len, in, s.length(), (unsigned char*)s.begin());
	if(outlen < 0)
		return nullptr;

	s.setLength(outlen);
	return s;
}
