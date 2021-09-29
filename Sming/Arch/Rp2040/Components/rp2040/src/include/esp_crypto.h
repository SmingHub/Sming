/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * esp_crypto.h - ROM / SDK crypto functions
 *
 * There are a bunch of routines available in the ESP8266 ROM and SDK which could save space,
 * and also potentially improving performance since ROM routines dont't require cache RAM.
 *
 * This header file provides prototypes so they can be used without conflicting with other libraries.
 *
 ****/

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * MD5
 */

#define MD5_SIZE 16

typedef struct {
	uint32_t state[4];
	uint32_t count; ///< Number of bits pushed
	uint32_t countHigh;
	uint8_t buffer[64];
} ESP_MD5_CTX;

void ESP_MD5_Init(ESP_MD5_CTX* context);
void ESP_MD5_Update(ESP_MD5_CTX* context, const void* input, uint32_t len);
void ESP_MD5_Final(uint8_t hash[], ESP_MD5_CTX* context);

/*
 * SHA1
 *
 * from: https://github.com/pvvx/esp8266web/blob/master/app/include/bios/cha1.h
 *
 */

#define SHA1_SIZE 20

typedef struct {
	uint32_t state[5];
	uint32_t count; ///< Number of bits pushed
	uint32_t countHigh;
	uint8_t buffer[64];
} ESP_SHA1_CTX;

void ESP_SHA1_Init(ESP_SHA1_CTX* context);
void ESP_SHA1_Update(ESP_SHA1_CTX* context, const uint8_t* input, size_t len);
void ESP_SHA1_Final(uint8_t digest[SHA1_SIZE], ESP_SHA1_CTX* context);

/*
 * HMAC
 *
 * From RTOS SDK
 *
 */

void ESP_hmac_md5(const uint8_t* key, int key_len, const uint8_t* data, int data_len, uint8_t* mac);
void ESP_hmac_md5_v(const uint8_t* key, int key_len, int num_elem, const uint8_t* addr[], const int* len, uint8_t* mac);

void ESP_hmac_sha1(const uint8_t* key, int key_len, const uint8_t* data, int data_len, uint8_t* mac);
void ESP_hmac_sha1_v(const uint8_t* key, int key_len, int num_elem, const uint8_t* addr[], const int* len,
					 uint8_t* mac);

/*
 * AES (Rijndael) cipher
 */

//PROVIDE ( aes_decrypt = 0x400092d4 );
//PROVIDE ( aes_decrypt_deinit = 0x400092e4 );
//PROVIDE ( aes_decrypt_init = 0x40008ea4 );
//PROVIDE ( aes_unwrap = 0x40009410 );
//PROVIDE(rijndaelKeySetupDec = 0x40008dd0);
//PROVIDE(rijndaelKeySetupEnc = 0x40009300);

#ifdef __cplusplus
}
#endif
