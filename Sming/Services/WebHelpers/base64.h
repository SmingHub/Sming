/*
 * 12/8/2018 (mikee47)
 *
 * 	Additional functions added to work with String objects for ease of use.
 */

#ifndef BASE64_H
#define BASE64_H

#include "WString.h"

/* encode binary data into base64 digits with MIME style === pads */
int base64_encode(size_t in_len, const unsigned char *in, size_t out_len, char *out);

String base64_encode(const unsigned char *in, size_t in_len);

static inline String base64_encode(const String& in)
{
	return base64_encode((unsigned char*)in.c_str(), in.length());
}


/* decode base64 digits with MIME style === pads into binary data */
int base64_decode(size_t in_len, const char *in, size_t out_len, unsigned char *out);

String base64_decode(const char *in, size_t in_len);


#endif /* BASE64_H */
