/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslInterface.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/
#pragma once

#include <WString.h>

/**
 * @ingroup ssl
 * @brief Encapsulates operations related to SSL certificates
 * @{
 */

/**
 * @brief List of distinguished names
 */
enum SslCertificateName {
	eSCN_CERT_COMMON_NAME,
	eSCN_CERT_ORGANIZATION,
	eSCN_CERT_ORGANIZATIONAL_NAME,
	eSCN_CERT_LOCATION,
	eSCN_CERT_COUNTRY,
	eSCN_CERT_STATE,
	eSCN_CA_CERT_COMMON_NAME,
	eSCN_CA_CERT_ORGANIZATION,
	eSCN_CA_CERT_ORGANIZATIONAL_NAME,
	eSCN_CA_CERT_LOCATION,
	eSCN_CA_CERT_COUNTRY,
	eSCN_CA_CERT_STATE
};

class SslCertificate
{
public:
	virtual ~SslCertificate()
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
	virtual const String getName(const SslCertificateName& name) const = 0;
};

/** @} */
