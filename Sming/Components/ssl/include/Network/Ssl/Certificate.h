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

/**
 * @brief X509 Relative Distinguished Name type
 *
 * From namespace 2.5.4.x
 */
#define SSL_X509_RDN_OID_MAP(XX)                                                                                       \
	XX(COMMON_NAME, 2, 5, 4, 3)                                                                                        \
	XX(SURNAME, 2, 5, 4, 4)                                                                                            \
	XX(SERIAL_NUMBER, 2, 5, 4, 5)                                                                                      \
	XX(COUNTRY_NAME, 2, 5, 4, 6)                                                                                       \
	XX(LOCALITY_NAME, 2, 5, 4, 7)                                                                                      \
	XX(STATE_OR_PROVINCE_NAME, 2, 5, 4, 8)                                                                             \
	XX(STREET_ADDRESS, 2, 5, 4, 9)                                                                                     \
	XX(ORGANIZATION_NAME, 2, 5, 4, 10)                                                                                 \
	XX(ORGANIZATIONAL_UNIT_NAME, 2, 5, 4, 11)                                                                          \
	XX(TITLE, 2, 5, 4, 12)                                                                                             \
	XX(BUSINESS_CATEGORY, 2, 5, 4, 15)                                                                                 \
	XX(POSTAL_ADDRESS, 2, 5, 4, 16)                                                                                    \
	XX(POSTAL_CODE, 2, 5, 4, 17)                                                                                       \
	XX(GIVEN_NAME, 2, 5, 4, 42)                                                                                        \
	XX(GENERATION_QUALIFIER, 2, 5, 4, 44)                                                                              \
	XX(X500_UNIQUE_IDENTIFIER, 2, 5, 4, 45)                                                                            \
	XX(DN_QUALIFIER, 2, 5, 4, 46)                                                                                      \
	XX(PSEUDONYM, 2, 5, 4, 65)

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
	 * @brief Distinguished Name type
	 */
	enum class DN {
		ISSUER,
		SUBJECT,
	};

	/**
	 * @brief Relative Distinguished Name type
	 */
	enum class RDN {
#define XX(tag, a, b, c, d) tag,
		SSL_X509_RDN_OID_MAP(XX)
#undef XX
			MAX
	};

	static String getRdnTypeString(Certificate::RDN rdn);

	virtual ~Certificate()
	{
	}

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
	virtual String getName(DN dn, RDN rdn) const = 0;

	size_t printTo(Print& p) const;
};

/** @} */

} // namespace Ssl

typedef Ssl::Certificate SslCertificate;
