/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 * 	Functions added to work with String objects for ease of use.
 *
 ****/

#ifndef BASE64_H
#define BASE64_H

#include "WString.h"

/** @brief encode binary data into base64 digits with MIME style === pads
 *  @param in_len quantity of characters to encode
 *  @param in data to encode
 *  @param out_len size of output buffer
 *  @param out buffer for base64-encoded text
 *  @retval int length of encoded text, or -1 if output buffer is too small
 *  @note Output is broken by newline every 72 characters. Final terminating newline
 *  is not included.
 */
int base64_encode(size_t in_len, const unsigned char* in, size_t out_len, char* out);

/** @brief encode a block of data into base64, stored in a String
 *  @param in text to encode
 *  @param in_len quantity of characters to encode
 *  @retval String the base64-encoded text
 */
String base64_encode(const unsigned char* in, size_t in_len);

/** @brief encode a block of data into base64, both input and output are String objects
 *  @param in
 *  @retval String
 */
static inline String base64_encode(const String& in)
{
	return base64_encode((unsigned char*)in.c_str(), in.length());
}

/** @brief decode base64 digits with MIME style === pads into binary data
 *  @param in_len length of source base64 text in characters
 *  @param in base64-encoded text
 *  @param out_len size of output buffer
 *  @param out buffer for decoded output
 *  @retval int length of decoded output, or -1 if output buffer is too small
 */
int base64_decode(size_t in_len, const char* in, size_t out_len, unsigned char* out);

/** @brief decode base64 text into binary data
 *  @param in source base64-encoded text
 *  @param in_len length of input text in characters
 *  @retval String the decoded text
 */
String base64_decode(const char* in, size_t in_len);

/** @brief encode a block of data into base64, both input and output are String objects
 *  @param in
 *  @retval String
 */
static inline String base64_decode(const String& in)
{
	return base64_decode(in.c_str(), in.length());
}

#endif /* BASE64_H */
