/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrError.cpp
 *
 ****/

#include <bearssl.h>
#include <Network/Ssl/Alert.h>
#include <FlashString/Map.hpp>

#define BR_ERROR_MAP(XX)                                                                                               \
	XX(BAD_PARAM, "Caller-provided parameter is incorrect.")                                                           \
	XX(BAD_STATE, "Operation requested by the caller cannot be applied with the current context state (e.g. "          \
				  "reading data while outgoing data is waiting to be sent).")                                          \
	XX(UNSUPPORTED_VERSION, "Incoming protocol or record version is unsupported.")                                     \
	XX(BAD_VERSION, "Incoming record version does not match the expected version.")                                    \
	XX(BAD_LENGTH, "Incoming record length is invalid.")                                                               \
	XX(TOO_LARGE,                                                                                                      \
	   "Incoming record is too large to be processed, or buffer is too small for the handshake message to send.")      \
	XX(BAD_MAC, "Decryption found an invalid padding, or the record MAC is not correct.")                              \
	XX(NO_RANDOM, "No initial entropy was provided, and none can be obtained from the OS.")                            \
	XX(UNKNOWN_TYPE, "Incoming record type is unknown.")                                                               \
	XX(UNEXPECTED, "Incoming record or message has wrong type with regards to the current engine state.")              \
	XX(BAD_CCS, "ChangeCipherSpec message from the peer has invalid contents.")                                        \
	XX(BAD_ALERT, "Alert message from the peer has invalid contents (odd length).")                                    \
	XX(BAD_HANDSHAKE, "Incoming handshake message decoding failed.")                                                   \
	XX(OVERSIZED_ID, "ServerHello contains a session ID which is larger than 32 bytes.")                               \
	XX(BAD_CIPHER_SUITE, "Server wants to use a cipher suite that we did not claim to support. This is also "          \
						 "reported if we tried to advertise a cipher suite that we do not support.")                   \
	XX(BAD_COMPRESSION, "Server wants to use a compression that we did not claim to support.")                         \
	XX(BAD_FRAGLEN, "Server's max fragment length does not match client's.")                                           \
	XX(BAD_SECRENEG, "Secure renegotiation failed.")                                                                   \
	XX(EXTRA_EXTENSION, "Server sent an extension type that we did not announce, or used the same extension type "     \
						"several times in a single ServerHello.")                                                      \
	XX(BAD_SNI, "Invalid Server Name Indication contents (when used by the server, this extension shall be empty).")   \
	XX(BAD_HELLO_DONE, "Invalid ServerHelloDone from the server (length is not 0).")                                   \
	XX(LIMIT_EXCEEDED, "Internal limit exceeded (e.g. server's public key is too large).")                             \
	XX(BAD_FINISHED, "Finished message from peer does not match the expected value.")                                  \
	XX(RESUME_MISMATCH, "Session resumption attempt with distinct version or cipher suite.")                           \
	XX(INVALID_ALGORITHM, "Unsupported or invalid algorithm (ECDHE curve, signature algorithm, hash function).")       \
	XX(BAD_SIGNATURE, "Invalid signature in ServerKeyExchange or CertificateVerify message.")                          \
	XX(WRONG_KEY_USAGE,                                                                                                \
	   "Peer's public key does not have the proper type or is not allowed for the requested operation.")               \
	XX(NO_CLIENT_AUTH,                                                                                                 \
	   "Client did not send a certificate upon request, or the client certificate could not be validated.")            \
	XX(IO, "I/O error or premature close on transport stream.")                                                        \
	XX(X509_INVALID_VALUE, "Invalid value in an ASN.1 structure.")                                                     \
	XX(X509_TRUNCATED, "Truncated certificate or other ASN.1 object.")                                                 \
	XX(X509_EMPTY_CHAIN, "Empty certificate chain XX( \no certificate at all).")                                       \
	XX(X509_INNER_TRUNC, "Decoding error: inner element extends beyond outer element size.")                           \
	XX(X509_BAD_TAG_CLASS, "Decoding error: unsupported tag class XX( \application or private).")                      \
	XX(X509_BAD_TAG_VALUE, "Decoding error: unsupported tag value.")                                                   \
	XX(X509_INDEFINITE_LENGTH, "Decoding error: indefinite length.")                                                   \
	XX(X509_EXTRA_ELEMENT, "Decoding error: extraneous element.")                                                      \
	XX(X509_UNEXPECTED, "Decoding error: unexpected element.")                                                         \
	XX(X509_NOT_CONSTRUCTED, "Decoding error: expected constructed element, but is primitive.")                        \
	XX(X509_NOT_PRIMITIVE, "Decoding error: expected primitive element, but is constructed.")                          \
	XX(X509_PARTIAL_BYTE, "Decoding error: BIT STRING length is not multiple of 8.")                                   \
	XX(X509_BAD_BOOLEAN, "Decoding error: BOOLEAN value has invalid length.")                                          \
	XX(X509_OVERFLOW, "Decoding error: value is off-limits.")                                                          \
	XX(X509_BAD_DN, "Invalid distinguished name.")                                                                     \
	XX(X509_BAD_TIME, "Invalid date/time representation.")                                                             \
	XX(X509_UNSUPPORTED, "Certificate contains unsupported features that cannot be ignored.")                          \
	XX(X509_LIMIT_EXCEEDED, "Key or signature size exceeds internal limits.")                                          \
	XX(X509_WRONG_KEY_TYPE, "Key type does not match that which was expected.")                                        \
	XX(X509_BAD_SIGNATURE, "Signature is invalid.")                                                                    \
	XX(X509_TIME_UNKNOWN, "Validation time is unknown.")                                                               \
	XX(X509_EXPIRED, "Certificate is expired or not yet valid.")                                                       \
	XX(X509_DN_MISMATCH, "Issuer/Subject DN mismatch in the chain.")                                                   \
	XX(X509_BAD_SERVER_NAME, "Expected server name was not found in the chain.")                                       \
	XX(X509_CRITICAL_EXTENSION, "Unknown critical extension in certificate.")                                          \
	XX(X509_NOT_CA, "Not a CA, or path length constraint violation.")                                                  \
	XX(X509_FORBIDDEN_KEY_USAGE, "Key Usage extension prohibits intended usage.")                                      \
	XX(X509_WEAK_PUBLIC_KEY, "Public key found in certificate is too small.")                                          \
	XX(X509_NOT_TRUSTED, "Chain could not be linked to a trust anchor.")

namespace Ssl
{
#define XX(tag, text) DEFINE_FSTR_LOCAL(errStr_##tag, #tag)
BR_ERROR_MAP(XX)
#undef XX

#define XX(tag, text) {BR_ERR_##tag, &errStr_##tag},
DEFINE_FSTR_MAP_LOCAL(errorMap, int, FSTR::String, BR_ERROR_MAP(XX));
#undef XX

String getErrorString(int error)
{
	if(error < 0) {
		error = -error;
	}
	if(error >= BR_ERR_SEND_FATAL_ALERT) {
		auto alert = Alert(error - BR_ERR_SEND_FATAL_ALERT);
		return F("SEND_") + getAlertString(alert);
	} else if(error >= BR_ERR_RECV_FATAL_ALERT) {
		auto alert = Alert(error - BR_ERR_RECV_FATAL_ALERT);
		return F("RECV_") + getAlertString(alert);
	} else {
		auto s = String(errorMap[error]);
		return s ?: F("Unknown_") + String(error);
	}
}

Alert getAlert(int error)
{
	if(error < 0) {
		error = -error;
	}
	if(error >= BR_ERR_SEND_FATAL_ALERT) {
		return Alert(error - BR_ERR_SEND_FATAL_ALERT);
	} else if(error >= BR_ERR_RECV_FATAL_ALERT) {
		return Alert(error - BR_ERR_RECV_FATAL_ALERT);
	} else {
		return Alert::Invalid;
	}
}

} // namespace Ssl
