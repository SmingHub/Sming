Cryptographic Support
=====================

Introduction
------------

Contains basic cryptographic support classes for Sming.

This provides a strongly typed and flexible C++ library for commonly used routines,
such as MD5, SHA hashes and HMAC.

It makes use of third-party libraries, such as axTLS, to provide the core support routines.
Architecture-specific ROM or SDK routines are used where appropriate to reduce code size
and improve performance.

The intention is that this library will provide the optimal implementations for any
given architecture but maintain a consistent interface and allow it to be easily extended.


Usage
-----

Add the following line to your application's `component.mk` file::

   COMPONENT_DEPENDS += crypto

Here's a basic example showing how to calculate a SHA1 hash on a C string::

   #include <Crypto/Sha1.h>

   void sha1Test()
   {
      // Returns a Crypto::Sha1::Hash object
      auto hash = Crypto::Sha1::calculate(buffer, strlen(buffer));
      Serial.print("SHA1: ");
      Serial.println(hash.toString());
   }

If your data has multiple chunks, use the longer form::

   void sha256Test(const String& s1, const String& s2)
   {
      Crypto::Sha256 ctx;
      ctx.update(s1);
      ctx.update(s2);
      Serial.print("SHA256: ");
      Serial.println(ctx.hash().toString());
   }

   sha256Test(F("This is some text to be hashed"), F("Hello"));

The `HMAC <https://en.wikipedia.org/wiki/HMAC>`__ algorithm is commonly used for verifying both
the integrity and authenticity of a message. It can be used with a variety of hashes, commonly
MD5 or SHA1. The library provides this as a templated class so can be used with any defined
hash class.

For example, an MD5 HMAC (as used with `CRAM-MD5 <https://en.wikipedia.org/wiki/CRAM-MD5>`__)
may be done like this::

   #include <Crypto/Hmac.h>
   #include <Crypto/Md5.h>

   void printHmacMd5(const String& data, const String& key)
   {
      auto hash = Crypto::Hmac<Crypto::Md5, 64>::calculate(data, key);
      Serial.print("HMAC.MD5 = ");
      Serial.println(hash.toString());
   }


.. toctree::

   api

