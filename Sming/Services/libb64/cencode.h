/*
cencode.h - c header for a base64 encoding algorithm

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64
*/

#ifndef BASE64_CENCODE_H
#define BASE64_CENCODE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	step_A, step_B, step_C
} base64_encodestep;

typedef struct
{
	base64_encodestep step;
	char result;
	int stepcount;
} base64_encodestate;


/** @brief Call first to initialise encoder
 *  @param state_in
 */
void base64_init_encodestate(base64_encodestate* state_in);

/** @brief Call as many times as required to encode text
 *  @param plaintext_in
 *  @param length_in
 *  @param code_out buffer must be at least (4 * length_in / 3) bytes
 *  @param state_in
 *  @retval unsigned number of bytes output
 */
unsigned base64_encode_block(const char* plaintext_in, int length_in, char* code_out, base64_encodestate* state_in);


/** @brief Call to finish encoding and return any remaining output bytes (maximum 2).
 *  @note This function does NOT append any extraneous characters (such as a carriage return) to the
 *  output text.
 */
unsigned base64_encode_blockend(char* code_out, base64_encodestate* state_in);

#ifdef __cplusplus
}
#endif

#endif /* BASE64_CENCODE_H */
