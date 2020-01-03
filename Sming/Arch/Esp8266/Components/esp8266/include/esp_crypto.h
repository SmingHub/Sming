#pragma once

#include <../third_party/include/ssl/ssl_crypto_misc.h>

#if 0

// Prevent conflict with other definitions
#define HEADER_CRYPTO_MISC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * MD5
 */

//PROVIDE ( MD5Final = 0x40009900 );
//PROVIDE ( MD5Init = 0x40009818 );
//PROVIDE ( MD5Update = 0x40009834 );

typedef struct {
	uint32_t i[2];
	uint32_t buf[4];
	unsigned char in[64];
	unsigned char digest[16];
} MD5_CTX;

void MD5Init(MD5_CTX* mdContext);
void MD5Update(MD5_CTX* mdContext, const unsigned char* inBuf, unsigned int inLen);
void MD5Final(unsigned char hash[], MD5_CTX* mdContext);

/*
 * SHA1
 *
 * from: https://github.com/pvvx/esp8266web/blob/master/app/include/bios/cha1.h
 *
 */

//PROVIDE(SHA1Final = 0x4000b648);
//PROVIDE(SHA1Init = 0x4000b584);
//PROVIDE(SHA1Transform = 0x4000a364);
//PROVIDE(SHA1Update = 0x4000b5a8);

#define SHA1_SIZE 20

typedef struct {
	uint32_t state[5];
	uint32_t count[2];
	uint8_t buffer[64];
	uint8_t extra[40];
} SHA1_CTX;

void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, const void* input, size_t len);
void SHA1Final(uint8_t digest[SHA1_SIZE], SHA1_CTX* context);

/*
 * SHA256
 */

#define SHA256_SIZE 32

typedef struct {
	uint32_t total[2];
	uint32_t state[8];
	uint8_t buffer[64];
} SHA256_CTX;

void SHA256_Init(SHA256_CTX* c);
void SHA256_Update(SHA256_CTX*, const void* input, size_t len);
void SHA256_Final(uint8_t* digest, SHA256_CTX*);

/*
 * AES
 */

//PROVIDE(aes_decrypt = 0x400092d4);
//PROVIDE(aes_decrypt_deinit = 0x400092e4);
//PROVIDE(aes_decrypt_init = 0x40008ea4);
//PROVIDE(aes_unwrap = 0x40009410);

/*
 * HMAC
 *
 * From RTOS SDK
 *
 */

//PROVIDE(hmac_md5 = 0x4000a2cc);
//PROVIDE(hmac_md5_vector = 0x4000a160);
//PROVIDE(hmac_sha1 = 0x4000ba28);
//PROVIDE(hmac_sha1_vector = 0x4000b8b4);

//int hmac_md5(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t* mac);
//int hmac_md5_vector(const uint8_t* key, size_t key_len, size_t num_elem, const uint8_t* addr[], const size_t* len,
//					uint8_t* mac);

/*
 * AES (Rijndael) cipher
 */

//PROVIDE(rijndaelKeySetupDec = 0x40008dd0);
//PROVIDE(rijndaelKeySetupEnc = 0x40009300);

#ifdef __cplusplus
}
#endif

#endif
