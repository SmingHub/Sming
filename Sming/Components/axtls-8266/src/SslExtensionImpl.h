/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxtlsExtension.h
 *
 * SSL Extension implementation based on axTLS library
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Ssl/SslInterface.h>
#include <axtls-8266/compat/lwipr_compat.h>

class SslExtensionImpl: public SslExtension
{
public:
	SslExtensionImpl();

   bool setHostName(const String& hostName) override;

   bool setMaxFragmentSize(uint8_t fragmentSize) override;

   void * getInternalObject() override
   {
	   return sslExtension;
   }

   virtual ~SslExtensionImpl()
   {
	   free(sslExtension);
	   sslExtension = nullptr;
   }

private:
   SSL_EXTENSIONS* sslExtension = nullptr;
};
