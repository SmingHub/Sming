/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Validator.h
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Delegate.h>
#include "Certificate.h"
#include "Fingerprints.h"

namespace Ssl
{
/**
 * @brief Base validator class
 *
 * Validation is performed by invoking each validator in turn until a successful
 * result is obtained.
 *
 * Custom validators may either override this class, or use a callback.
 */
class Validator
{
public:
	virtual ~Validator()
	{
	}

	virtual bool validate(const Certificate& certificate) = 0;
};

/**
 * @brief Class template to validate any kind of fingerprint
 * @tparam Fingerprint The Fingerprint type
 */
template <class Fingerprint> class FingerprintValidator : public Validator
{
public:
	FingerprintValidator(const Fingerprint& fingerprint) : fp(fingerprint)
	{
	}

	bool validate(const Certificate& certificate) override
	{
		Fingerprint certFp;
		return certificate.getFingerprint(certFp) && certFp.hash == fp.hash;
	}

private:
	Fingerprint fp;
};

/** @brief Validator callback function
 *  @param ssl Contains certificate to validate (may be NULL)
 *  @param data Data for the callback to use
 *  @retval bool true if validation succeeded
 *  @note Callback must ALWAYS release any allocated memory before returning.
 *  If called with certificate = NULL then just release memory and return false.
 */
using ValidatorCallback = Delegate<bool(const Certificate* certificate, void* data)>;

/**
 * @brief Validator class wrapping a user-provided callback delegate, plus optional parameter
 */
class CallbackValidator : public Validator
{
public:
	CallbackValidator(ValidatorCallback callback, void* param) : callback(callback), param(param)
	{
	}

	~CallbackValidator()
	{
		// If callback hasn't been invoked yet, do it now to release any allocated memory
		if(callback) {
			callback(nullptr, param);
		}
	}

	bool validate(const Certificate& certificate) override
	{
		bool res = callback ? callback(&certificate, param) : false;
		// Only invoke the callback once
		callback = nullptr;
		return res;
	}

private:
	ValidatorCallback callback;
	void* param;
};

} // namespace Ssl

/**
 * @deprecated Use `Ssl::ValidatorCallback` instead
 */
typedef Ssl::ValidatorCallback SslValidatorCallback SMING_DEPRECATED;
