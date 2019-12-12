/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslInterface.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/
#pragma once

// crypto library
#define MD5_SIZE 16

/* errors that can be generated */
#define SSL_OK                                  0
#define SSL_NOT_OK                              -1
#define SSL_ERROR_DEAD                          -2
#define SSL_CLOSE_NOTIFY                        -3
#define SSL_ERROR_CONN_LOST                     -256
#define SSL_ERROR_RECORD_OVERFLOW               -257
#define SSL_ERROR_SOCK_SETUP_FAILURE            -258
#define SSL_ERROR_INVALID_HANDSHAKE             -260
#define SSL_ERROR_INVALID_PROT_MSG              -261
#define SSL_ERROR_INVALID_HMAC                  -262
#define SSL_ERROR_INVALID_VERSION               -263
#define SSL_ERROR_UNSUPPORTED_EXTENSION         -264
#define SSL_ERROR_INVALID_SESSION               -265
#define SSL_ERROR_NO_CIPHER                     -266
#define SSL_ERROR_INVALID_CERT_HASH_ALG         -267
#define SSL_ERROR_BAD_CERTIFICATE               -268
#define SSL_ERROR_INVALID_KEY                   -269
#define SSL_ERROR_FINISHED_INVALID              -271
#define SSL_ERROR_NO_CERT_DEFINED               -272
#define SSL_ERROR_NO_CLIENT_RENOG               -273
#define SSL_ERROR_NOT_SUPPORTED                 -274
#define SSL_X509_OFFSET                         -512
#define SSL_X509_ERROR(A)                       (SSL_X509_OFFSET+A)

/* alert types that are recognized */
#define SSL_ALERT_TYPE_WARNING                  1
#define SLL_ALERT_TYPE_FATAL                    2

/* these are all the alerts that are recognized */
#define SSL_ALERT_CLOSE_NOTIFY                  0
#define SSL_ALERT_UNEXPECTED_MESSAGE            10
#define SSL_ALERT_BAD_RECORD_MAC                20
#define SSL_ALERT_RECORD_OVERFLOW               22
#define SSL_ALERT_HANDSHAKE_FAILURE             40
#define SSL_ALERT_BAD_CERTIFICATE               42
#define SSL_ALERT_UNSUPPORTED_CERTIFICATE       43
#define SSL_ALERT_CERTIFICATE_EXPIRED           45
#define SSL_ALERT_CERTIFICATE_UNKNOWN           46
#define SSL_ALERT_ILLEGAL_PARAMETER             47
#define SSL_ALERT_UNKNOWN_CA                    48
#define SSL_ALERT_DECODE_ERROR                  50
#define SSL_ALERT_DECRYPT_ERROR                 51
#define SSL_ALERT_INVALID_VERSION               70
#define SSL_ALERT_NO_RENEGOTIATION              100
#define SSL_ALERT_UNSUPPORTED_EXTENSION         110

// SSL Options
#define SSL_CLIENT_AUTHENTICATION               0x00010000
#define SSL_SERVER_VERIFY_LATER                 0x00020000
#define SSL_NO_DEFAULT_KEY                      0x00040000
#define SSL_DISPLAY_STATES                      0x00080000
#define SSL_DISPLAY_BYTES                       0x00100000
#define SSL_DISPLAY_CERTS                       0x00200000
#define SSL_DISPLAY_RSA                         0x00400000

/* X.509/X.520 distinguished name types */
#define SSL_X509_CERT_COMMON_NAME               0
#define SSL_X509_CERT_ORGANIZATION              1
#define SSL_X509_CERT_ORGANIZATIONAL_NAME       2
#define SSL_X509_CERT_LOCATION                  3
#define SSL_X509_CERT_COUNTRY                   4
#define SSL_X509_CERT_STATE                     5
#define SSL_X509_CA_CERT_COMMON_NAME            6
#define SSL_X509_CA_CERT_ORGANIZATION           7
#define SSL_X509_CA_CERT_ORGANIZATIONAL_NAME    8
#define SSL_X509_CA_CERT_LOCATION               9
#define SSL_X509_CA_CERT_COUNTRY                10
#define SSL_X509_CA_CERT_STATE                  11

/* SSL object loader types */
#define SSL_OBJ_X509_CERT                       1
#define SSL_OBJ_X509_CACERT                     2
#define SSL_OBJ_RSA_KEY                         3
#define SSL_OBJ_PKCS8                           4
#define SSL_OBJ_PKCS12                          5
