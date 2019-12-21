/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Certificate.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/
#pragma once

#include <WString.h>

namespace Ssl
{
/**
 * @ingroup ssl
 * @brief Encapsulates operations related to SSL certificates
 * @{
 */

class Certificate
{
public:
	/**
	 * @brief List of distinguished names
	 * @note Defined by AXTLS, other implementations will need to translate
	 */
	enum class Name {
		CERT_COMMON_NAME,
		CERT_ORGANIZATION,
		CERT_ORGANIZATIONAL_NAME,
		CERT_LOCATION,
		CERT_COUNTRY,
		CERT_STATE,
		CA_CERT_COMMON_NAME,
		CA_CERT_ORGANIZATION,
		CA_CERT_ORGANIZATIONAL_NAME,
		CA_CERT_LOCATION,
		CA_CERT_COUNTRY,
		CA_CERT_STATE,
	};

	virtual ~Certificate()
	{
	}

	/**
	 * @brief Check we have a valid certificate
	 */
	virtual bool isValid() const = 0;

	/**
    * @brief Check if certificate fingerprint (SHA1) matches the one given.
    * @param hash - SHA1 fingerprint to match against
    * @retval bool true on match, false otherwise
    */
	virtual bool matchFingerprint(const uint8_t* hash) const = 0;

	/**
	* @brief Check if SHA256 hash of Subject Public Key Info matches the one given.
    * @param hash SHA256 hash to match against
    * @retval bool true on match, false otherwise
    */
	virtual bool matchPki(const uint8_t* hash) const = 0;

	/**
   * @brief Retrieve an X.509 distinguished name component
   * @param name the desired distinguished name
   * @retval the value for the desired distinguished name
   */
	virtual const String getName(Name name) const = 0;
};

/** @} */

} // namespace Ssl

typedef Ssl::Certificate SslCertificate;
