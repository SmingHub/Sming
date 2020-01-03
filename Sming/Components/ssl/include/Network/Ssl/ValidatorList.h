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

#include "Validator.h"
#include "Fingerprints.h"
#include <WVector.h>

namespace Ssl
{
/**
 * @brief Performs certificate validation
 */
class ValidatorList : public Vector<Validator>
{
public:
	bool add(Validator* validator)
	{
		return validator ? Vector::addElement(validator) : false;
	}

	/**
	 * @brief Pin a fingerprint
	 */
	template <class T> bool pin(const T& fingerprint)
	{
		return add(new FingerprintValidator<T>(fingerprint));
	}

	/**
	 * @brief Register a custom validator callback
	 * @param callback
	 * @param data User-provided data (optional)
	 */
	bool add(ValidatorCallback callback, void* data = nullptr)
	{
		return add(new CallbackValidator(callback, data));
	}

	/**
	 * @brief Validate certificate via registered validators
	 * @param certificate When called with nullptr will free all validators, then fail
	 * @retval bool  true on success, false on failure
	 *
	 * We only need one match for a successful result, but we free all the validators.
	 * This method must be called no more than ONCE.
	 */
	bool validate(const Certificate* certificate);
};

} // namespace Ssl
