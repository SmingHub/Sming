COMPONENT_SUBMODULES	:= libsodium

COMPONENT_SRCFILES := \
	crypto_aead/chacha20poly1305/sodium/aead_chacha20poly1305.c \
	crypto_aead/xchacha20poly1305/sodium/aead_xchacha20poly1305.c \
	crypto_auth/crypto_auth.c \
	crypto_auth/hmacsha256/auth_hmacsha256.c \
	crypto_auth/hmacsha512/auth_hmacsha512.c \
	crypto_auth/hmacsha512256/auth_hmacsha512256.c \
	crypto_box/crypto_box.c \
	crypto_box/crypto_box_easy.c \
	crypto_box/crypto_box_seal.c \
	crypto_box/curve25519xsalsa20poly1305/box_curve25519xsalsa20poly1305.c \
	crypto_core/ed25519/ref10/ed25519_ref10.c \
	crypto_core/hchacha20/core_hchacha20.c \
	crypto_core/hsalsa20/ref2/core_hsalsa20_ref2.c \
	crypto_core/hsalsa20/core_hsalsa20.c \
	crypto_core/salsa/ref/core_salsa_ref.c \
	crypto_generichash/crypto_generichash.c \
	crypto_generichash/blake2b/generichash_blake2.c \
	crypto_generichash/blake2b/ref/blake2b-compress-ref.c \
	crypto_generichash/blake2b/ref/blake2b-ref.c \
	crypto_generichash/blake2b/ref/generichash_blake2b.c \
	crypto_hash/crypto_hash.c \
	crypto_hash/sha256/hash_sha256.c \
	crypto_hash/sha256/cp/hash_sha256_cp.c \
	crypto_hash/sha512/hash_sha512.c \
	crypto_hash/sha512/cp/hash_sha512_cp.c \
	crypto_kdf/blake2b/kdf_blake2b.c \
	crypto_kdf/crypto_kdf.c \
	crypto_kx/crypto_kx.c \
	crypto_onetimeauth/crypto_onetimeauth.c \
	crypto_onetimeauth/poly1305/onetimeauth_poly1305.c \
	crypto_onetimeauth/poly1305/donna/poly1305_donna.c \
	crypto_pwhash/argon2/argon2-core.c \
	crypto_pwhash/argon2/argon2-encoding.c \
	crypto_pwhash/argon2/argon2-fill-block-ref.c \
	crypto_pwhash/argon2/argon2.c \
	crypto_pwhash/argon2/blake2b-long.c \
	crypto_pwhash/argon2/pwhash_argon2i.c \
	crypto_pwhash/argon2/pwhash_argon2id.c \
	crypto_pwhash/crypto_pwhash.c \
	crypto_scalarmult/crypto_scalarmult.c \
	crypto_scalarmult/curve25519/ref10/x25519_ref10.c \
	crypto_scalarmult/curve25519/scalarmult_curve25519.c \
	crypto_secretbox/crypto_secretbox.c \
	crypto_secretbox/crypto_secretbox_easy.c \
	crypto_secretbox/xsalsa20poly1305/secretbox_xsalsa20poly1305.c \
	crypto_secretstream/xchacha20poly1305/secretstream_xchacha20poly1305.c \
	crypto_shorthash/crypto_shorthash.c \
	crypto_shorthash/siphash24/shorthash_siphash24.c \
	crypto_shorthash/siphash24/ref/shorthash_siphash24_ref.c \
	crypto_sign/crypto_sign.c \
	crypto_sign/ed25519/sign_ed25519.c \
	crypto_sign/ed25519/ref10/keypair.c \
	crypto_sign/ed25519/ref10/open.c \
	crypto_sign/ed25519/ref10/sign.c \
	crypto_stream/chacha20/stream_chacha20.c \
	crypto_stream/chacha20/ref/chacha20_ref.c \
	crypto_stream/salsa20/ref/salsa20_ref.c \
	crypto_stream/crypto_stream.c \
	crypto_stream/salsa20/stream_salsa20.c \
	crypto_stream/xsalsa20/stream_xsalsa20.c \
	crypto_verify/sodium/verify.c \
	randombytes/randombytes.c \
	sodium/codecs.c \
	sodium/core.c \
	sodium/runtime.c \
	sodium/utils.c \
	sodium/version.c \
	crypto_box/curve25519xchacha20poly1305/box_curve25519xchacha20poly1305.c \
	crypto_box/curve25519xchacha20poly1305/box_seal_curve25519xchacha20poly1305.c \
	crypto_core/ed25519/core_ed25519.c \
	crypto_core/ed25519/core_ristretto255.c \
	crypto_pwhash/scryptsalsa208sha256/crypto_scrypt-common.c \
	crypto_pwhash/scryptsalsa208sha256/scrypt_platform.c \
	crypto_pwhash/scryptsalsa208sha256/pbkdf2-sha256.c \
	crypto_pwhash/scryptsalsa208sha256/pwhash_scryptsalsa208sha256.c \
	crypto_pwhash/scryptsalsa208sha256/nosse/pwhash_scryptsalsa208sha256_nosse.c \
	crypto_scalarmult/ed25519/ref10/scalarmult_ed25519_ref10.c \
	crypto_scalarmult/ristretto255/ref10/scalarmult_ristretto255_ref10.c \
	crypto_secretbox/xchacha20poly1305/secretbox_xchacha20poly1305.c \
	crypto_shorthash/siphash24/shorthash_siphashx24.c \
	crypto_shorthash/siphash24/ref/shorthash_siphashx24_ref.c \
	crypto_sign/ed25519/ref10/obsolete.c \
	crypto_stream/salsa2012/ref/stream_salsa2012_ref.c \
	crypto_stream/salsa2012/stream_salsa2012.c \
	crypto_stream/salsa208/ref/stream_salsa208_ref.c \
	crypto_stream/salsa208/stream_salsa208.c \
	crypto_stream/xchacha20/stream_xchacha20.c \
	randombytes/sysrandom/randombytes_sysrandom.c

COMPONENT_SRCFILES	:= $(addprefix libsodium/src/libsodium/,$(COMPONENT_SRCFILES))
COMPONENT_INCDIRS := \
	include \
	libsodium/src/libsodium/include
EXTRA_INCDIR		:= libsodium/src/libsodium/include/sodium

# results from configure script run for --host=xtensa-lx106-elf (unused defines removed)
COMPONENT_CFLAGS := \
	-DSODIUM_STATIC \
	-DHAVE_C_VARARRAYS=1 \
	-DNATIVE_LITTLE_ENDIAN=1 \
	-DHAVE_INLINE_ASM=1 \
	-DHAVE_WEAK_SYMBOLS=1 \
	-DHAVE_ALLOCA_H=1 \
	-DHAVE_ALLOCA=1 \
	-DCONFIGURED=1 \
	-D_GNU_SOURCE=1

COMPONENT_CFLAGS += \
	-Wno-unused-function \
	-Wno-unknown-pragmas

COMPONENT_CXXFLAGS := $(COMPONENT_CFLAGS)
