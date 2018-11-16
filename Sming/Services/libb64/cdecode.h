/*
cdecode.h - c header for a base64 decoding algorithm

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64
*/

#ifndef BASE64_CDECODE_H
#define BASE64_CDECODE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	step_a, step_b, step_c, step_d
} base64_decodestep;

typedef struct
{
	base64_decodestep step;
	char plainchar;
} base64_decodestate;

/** @brief Call first to initialise decoder
 *  @param state_in
 */
void base64_init_decodestate(base64_decodestate* state_in);

/** @brief Call as many times as required to decode text
 *  @param code_in
 *  @param length_in
 *  @param plaintext_out may use same buffer as code_in if required
 *  @param state_in
 *  @retval unsigned number of bytes output
 */
unsigned base64_decode_block(const char* code_in, const int length_in, char* plaintext_out, base64_decodestate* state_in);

#ifdef __cplusplus
}
#endif

#endif /* BASE64_CDECODE_H */
