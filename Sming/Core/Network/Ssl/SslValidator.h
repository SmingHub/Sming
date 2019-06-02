/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslValidator.h
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

/** @defgroup tcp TCP
 *  @ingroup networking
 *  @{
 */

#pragma once

#include "ssl/ssl.h"
#include "ssl/tls1.h"

#include <functional>
#include "WVector.h"

#include "SslFingerprints.h"

/** @brief Validator callback function
 *  @param ssl Contains certificate to validate (may be NULL)
 *  @param data Data for the callback to use
 *  @retval bool true if validation succeeded
 *  @note Callback must ALWAYS release any allocate memory before returning.
 *  If called with ssl = NULL then just release memory and return false.
 */
typedef std::function<bool(SSL* ssl, void* data)> SslValidatorCallback;

struct SslValidator {
	SslValidatorCallback callback;
	void* data; ///< Callback-specific data, e.g. fingerprint to compare against
};

class SslValidatorList : private Vector<SslValidator>
{
public:
	~SslValidatorList()
	{
		// Make sure memory gets released
		validate(nullptr);
	}

	bool add(SslValidatorCallback callback, void* data)
	{
		return Vector::add(SslValidator{callback, data});
	}

	/**
	 * @brief	Add a standard fingerprint validator
	 * @param	fingerprint	The fingerprint data against which the match should be performed.
	 * 						Must be allocated on the heap and will be deleted after use.
	 * @param	type	The fingerprint type - see SslFingerprintType for details.
	 * @retval	bool	true on success, false on failure
	 */
	bool add(const uint8_t* fingerprint, SslFingerprintType type);

	/**
	 * @brief	Add validators for standard fingerprints
	 * @param	fingerprints Will be invalid after returning as data is moved rather than copied
	 * @retval	bool true on success, false on failure
	 */
	bool add(SslFingerprints& fingerprints);

	/** @brief Used to validate certificate by invoking each validator callback until successful
	 *  @param ssl When called with nullptr will simply de-allocate any validator memory
	 * @retval bool  true on success, false on failure
	 */
	bool validate(SSL* ssl);
};

/** @} */
