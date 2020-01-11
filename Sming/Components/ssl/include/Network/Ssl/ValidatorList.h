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
 *
 * Validators are created in the application's session initialisation callback.
 * When the certificate has been received, it is checked against each registered
 * validator in turn until successful.
 * All validators are destroyed during this process.
 *
 * If there are no validators in the list then the certificate will not be checked
 * and the connection accepted.
 */
class ValidatorList : public Vector<Validator>
{
public:
	/**
	 * @brief Add a validator to the list
	 * @param validator Must be allocated on the heap
	 */
	bool add(Validator* validator)
	{
		return validator ? Vector::addElement(validator) : false;
	}

	/**
	 * @brief Pin a fingerprint
	 *
	 * Creates and adds a fingerprint validator to the list
	 */
	template <class T> bool pin(const T& fingerprint)
	{
		if(!add(new FingerprintValidator<T>(fingerprint))) {
			return false;
		}

		fingerprintTypes.add(fingerprint.type);
		return true;
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
	 * @note Called by SSL framework.
	 *
	 * We only need one match for a successful result, but we free all the validators.
	 * This method must be called no more than ONCE.
	 */
	bool validate(const Certificate* certificate);

	/**
	 * @brief Contains a list of registered fingerprint types
	 *
	 * Allows implementations to avoid calculating fingerprint values
	 * which are not required, as this is computationally expensive.
	 */
	Fingerprint::Types fingerprintTypes;
};

} // namespace Ssl
