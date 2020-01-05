/*
 * Contains used ROM functions, renamed if necessary to avoid conflict with other C libraries
 */

/*
 * MD5
 */

PROVIDE(ESP_MD5_Final = 0x40009900);
PROVIDE(ESP_MD5_Init = 0x40009818);
PROVIDE(ESP_MD5_Update = 0x40009834);

/*
 * SHA1
 */

PROVIDE(ESP_SHA1_Final = 0x4000b648);
PROVIDE(ESP_SHA1_Init = 0x4000b584);
PROVIDE(ESP_SHA1_Transform = 0x4000a364);
PROVIDE(ESP_SHA1_Update = 0x4000b5a8);

/*
 * AES
 */

/*
PROVIDE(aes_decrypt = 0x400092d4);
PROVIDE(aes_decrypt_deinit = 0x400092e4);
PROVIDE(aes_decrypt_init = 0x40008ea4);
PROVIDE(aes_unwrap = 0x40009410);
*/

/*
 * HMAC
 *
 * From RTOS SDK
 *
 */

/*
PROVIDE(hmac_md5 = 0x4000a2cc);
PROVIDE(hmac_md5_vector = 0x4000a160);

PROVIDE(hmac_sha1 = 0x4000ba28);
PROVIDE(hmac_sha1_vector = 0x4000b8b4);
*/

/*
 * AES (Rijndael) cipher
 */

/*
PROVIDE(rijndaelKeySetupDec = 0x40008dd0);
PROVIDE(rijndaelKeySetupEnc = 0x40009300);
*/
