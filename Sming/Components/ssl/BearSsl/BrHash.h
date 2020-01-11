/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrHash.h - Support use of crypto library with BearSSL
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#include <bearssl.h>
#include <Crypto/HashEngine.h>

namespace Ssl
{
/**
 * @brief Class template to wrap crypto library hash implementations for BearSSL
 */
template <class HashEngine, uint32_t desc> struct BrHashContext {
public:
#define GET_SELF() auto self = reinterpret_cast<BrHashContext*>(const_cast<br_hash_class**>(ctx))

	static void br_init(const br_hash_class** ctx)
	{
		GET_SELF();
		self->vtable = &__fstr__vt;
		self->eng.init();
	}

	static void br_update(const br_hash_class** ctx, const void* data, size_t len)
	{
		GET_SELF();
		self->eng.update(data, len);
	}

	static void br_out(const br_hash_class* const* ctx, void* dst)
	{
		GET_SELF();
		self->eng.final(static_cast<uint8_t*>(dst));
	}

	static uint64_t br_state(const br_hash_class* const* ctx, void* dst)
	{
		GET_SELF();
		return self->eng.get_state(dst);
	}

	static void br_set_state(const br_hash_class** ctx, const void* stb, uint64_t count)
	{
		GET_SELF();
		self->eng.set_state(stb, count);
	}

#undef GET_SELF

	const br_hash_class* vtable;
	HashEngine eng;
	static const br_hash_class __fstr__vt;
};

template <class HashEngine, uint32_t desc>
const br_hash_class BrHashContext<HashEngine, desc>::__fstr__vt PROGMEM = {
	sizeof(BrHashContext<HashEngine, 0>),
	BR_HASHDESC_OUT(HashEngine::hashsize) | BR_HASHDESC_STATE(HashEngine::statesize) | desc,
	br_init,
	br_update,
	br_out,
	br_state,
	br_set_state,
};

using Md5Context =
	BrHashContext<Crypto::Md5Engine, BR_HASHDESC_ID(br_md5_ID) | BR_HASHDESC_LBLEN(6) | BR_HASHDESC_MD_PADDING>;

using Sha1Context = BrHashContext<Crypto::Sha1Engine, BR_HASHDESC_ID(br_sha1_ID) | BR_HASHDESC_LBLEN(6) |
														  BR_HASHDESC_MD_PADDING | BR_HASHDESC_MD_PADDING_BE>;

using Sha224Context = BrHashContext<Crypto::Sha224Engine, BR_HASHDESC_ID(br_sha224_ID) | BR_HASHDESC_LBLEN(6) |
															  BR_HASHDESC_MD_PADDING | BR_HASHDESC_MD_PADDING_BE>;

using Sha256Context = BrHashContext<Crypto::Sha256Engine, BR_HASHDESC_ID(br_sha256_ID) | BR_HASHDESC_LBLEN(6) |
															  BR_HASHDESC_MD_PADDING | BR_HASHDESC_MD_PADDING_BE>;

using Sha384Context =
	BrHashContext<Crypto::Sha384Engine, BR_HASHDESC_ID(br_sha384_ID) | BR_HASHDESC_LBLEN(7) | BR_HASHDESC_MD_PADDING |
											BR_HASHDESC_MD_PADDING_BE | BR_HASHDESC_MD_PADDING_128>;

using Sha512Context =
	BrHashContext<Crypto::Sha256Engine, BR_HASHDESC_ID(br_sha512_ID) | BR_HASHDESC_LBLEN(7) | BR_HASHDESC_MD_PADDING |
											BR_HASHDESC_MD_PADDING_BE | BR_HASHDESC_MD_PADDING_128>;

} // namespace Ssl
