/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SignEd25519.cpp
 *
 ****/

#include <Sodium/SignEd25519.h>
#include <FlashString/String.hpp>
#include <crypto_verify_32.h>

extern "C" {
#include <sodium/private/ed25519_ref10.h>
}

namespace Sodium
{
/*
 * See _crypto_sign_ed25519_verify_detached for original implementation
 */
bool SignEd25519::verify(const Signature& sig, const PrivateKey& key)
{
#ifdef ED25519_COMPAT
	if(sig[63] & 224) {
		return false;
	}
#else
	if(sc25519_is_canonical(&sig[32]) == 0 || ge25519_has_small_order(sig.data()) != 0) {
		return false;
	}
	if(ge25519_is_canonical(key.data()) == 0 || ge25519_has_small_order(key.data()) != 0) {
		return false;
	}
#endif

	ge25519_p3 A;
	if(ge25519_frombytes_negate_vartime(&A, key.data()) != 0) {
		return false;
	}

	DEFINE_FSTR_LOCAL(DOM2PREFIX, "SigEd25519 "
								  "no "
								  "Ed25519 "
								  "collisions"
								  "\x01\x00");

	auto m = getHash();

	reset();
	update(DOM2PREFIX);
	update(sig.data(), 32);
	update(key);
	update(m);
	auto h = getHash();
	sc25519_reduce(h.data());

	ge25519_p2 R;
	ge25519_double_scalarmult_vartime(&R, h.data(), &A, &sig[32]);
	Crypto::ByteArray<32> rcheck;
	ge25519_tobytes(rcheck.data(), &R);

	return (crypto_verify_32(rcheck.data(), sig.data()) == 0) && (memcmp(rcheck.data(), sig.data(), 32) == 0);
}

} // namespace Sodium
