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
#include <WVector.h>

#include "Fingerprints.h"
#include "Certificate.h"

namespace Ssl
{
/**
 * @brief Defines a validator instance
 */
struct Validator {
	/** @brief Validator callback function
	 *  @param ssl Contains certificate to validate (may be NULL)
	 *  @param data Data for the callback to use
	 *  @retval bool true if validation succeeded
	 *  @note Callback must ALWAYS release any allocate memory before returning.
	 *  If called with certificate = NULL then just release memory and return false.
	 */
	using Callback = Delegate<bool(const Certificate* certificate, void* data)>;

	Callback callback;
	void* data; ///< Callback-specific data, e.g. fingerprint to compare against
};

/**
 * @brief List of validators to perform certificate checking
 */
class ValidatorList : private Vector<Validator>
{
public:
	~ValidatorList()
	{
		// Make sure memory gets released
		validate(nullptr);
	}

	bool add(Validator::Callback callback, void* data)
	{
		return Vector::add(Validator{callback, data});
	}

	/**
	 * @brief	Add a standard fingerprint validator
	 * @param	fingerprint	The fingerprint data against which the match should be performed.
	 * 						Must be allocated on the heap and will be deleted after use.
	 * @param	type	The fingerprint type - see FingerprintType for details.
	 * @retval	bool	true on success, false on failure
	 */
	bool add(const uint8_t* fingerprint, FingerprintType type);

	/**
	 * @brief	Add validators for standard fingerprints
	 * @param	fingerprints Will be invalid after returning as data is moved rather than copied
	 * @retval	bool true on success, false on failure
	 */
	bool add(Fingerprints& fingerprints);

	/** @brief Used to validate certificate by invoking each validator callback until successful
	 *  @param certificate When called with nullptr will simply de-allocate any validator memory
	 * @retval bool  true on success, false on failure
	 */
	bool validate(const Certificate* certificate);

	using Vector::count;
};

} // namespace Ssl

/**
 * @deprecated Use `Ssl::Validator::Callback` instead
 */
typedef Ssl::Validator::Callback SslValidatorCallback SMING_DEPRECATED;
