/* base64.h : base-64 / MIME encode/decode */
/* PUBLIC DOMAIN - Jon Mayo - November 13, 2003 */
/* $Id: base64.h 128 2007-04-20 08:20:40Z orange $ */
#ifndef BASE64_H
#define BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

#include <user_config.h>

/* used to encode 3 bytes into 4 base64 digits */
void base64encode(const unsigned char in[3], unsigned char out[4], int count);

/* used to decode 4 base64 digits into 3 bytes */
int base64decode(const char in[4], char out[3]);

/* encode binary data into base64 digits with MIME style === pads */
int base64_encode(size_t in_len, const unsigned char *in, size_t out_len, char *out);

/* decode base64 digits with MIME style === pads into binary data */
int base64_decode(size_t in_len, const char *in, size_t out_len, unsigned char *out);

#ifdef __cplusplus
}
#endif

#endif /* BASE64_H */
