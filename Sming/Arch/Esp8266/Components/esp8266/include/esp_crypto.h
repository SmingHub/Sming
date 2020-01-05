/*
 * There are a bunch of routines available in the ESP8266 ROM which could save space.
 * This header file is intended to provide prototypes so they can be used.
 *
 * It's not just a question of saving space, but also potentially improving performance since
 * the routines won't require cache space.
 *
 * Definitely a work-in-progress. Not currently used.
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * MD5
 */

typedef struct {
	uint32_t i[2];
	uint32_t buf[4];
	unsigned char in[64];
	unsigned char digest[16];
} ESP_MD5_CTX;

void ESP_MD5_Init(ESP_MD5_CTX* mdContext);
void ESP_MD5_Update(ESP_MD5_CTX* mdContext, const unsigned char* inBuf, unsigned int inLen);
void ESP_MD5_Final(unsigned char hash[], ESP_MD5_CTX* mdContext);

/*
 * SHA1
 *
 * from: https://github.com/pvvx/esp8266web/blob/master/app/include/bios/cha1.h
 *
 */

#define SHA1_SIZE 20

typedef struct {
	uint32_t state[5];
	uint32_t count[2];
	uint8_t buffer[64];
	uint8_t extra[40];
} ESP_SHA1_CTX;

void ESP_SHA1_Init(ESP_SHA1_CTX* context);
void ESP_SHA1_Update(ESP_SHA1_CTX* context, const void* input, size_t len);
void ESP_SHA1_Final(uint8_t digest[SHA1_SIZE], ESP_SHA1_CTX* context);

/*
 * SHA256
 */

#define SHA256_SIZE 32

typedef struct {
	uint32_t total[2];
	uint32_t state[8];
	uint8_t buffer[64];
} ESP_SHA256_CTX;

void ESP_SHA256_Init(ESP_SHA256_CTX* c);
void ESP_SHA256_Update(ESP_SHA256_CTX*, const void* input, size_t len);
void ESP_SHA256_Final(uint8_t* digest, ESP_SHA256_CTX*);

/*
 * AES
 */

//PROVIDE(ESP_aes_decrypt = 0x400092d4);
//PROVIDE(ESP_aes_decrypt_deinit = 0x400092e4);
//PROVIDE(ESP_aes_decrypt_init = 0x40008ea4);
//PROVIDE(ESP_aes_unwrap = 0x40009410);

/*
 * HMAC
 *
 * From RTOS SDK
 *
 */

//PROVIDE(ESP_hmac_md5 = 0x4000a2cc);
//PROVIDE(ESP_hmac_md5_vector = 0x4000a160);

//PROVIDE(ESP_hmac_sha1 = 0x4000ba28);
//PROVIDE(ESP_hmac_sha1_vector = 0x4000b8b4);

int ESP_hmac_md5(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t* mac);
int ESP_hmac_md5_vector(const uint8_t* key, size_t key_len, size_t num_elem, const uint8_t* addr[], const size_t* len,
						uint8_t* mac);

int ESP_hmac_sha1(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t* mac);
int ESP_hmac_sha1_vector(const uint8_t* key, size_t key_len, size_t num_elem, const uint8_t* addr[], const size_t* len,
						 uint8_t* mac);

/*
 * AES (Rijndael) cipher
 */

//PROVIDE(rijndaelKeySetupDec = 0x40008dd0);
//PROVIDE(rijndaelKeySetupEnc = 0x40009300);

#ifdef __cplusplus
}
#endif
