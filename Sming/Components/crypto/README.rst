Cryptographic Support
=====================

Introduction
------------

Contains basic cryptographic support classes for Sming.

This provides a strongly typed and flexible C++ library for commonly used routines,
such as MD5, SHA hashes and HMAC.

Architecture-specific ROM or SDK routines are used where appropriate to reduce code size
and improve performance.

The intention is that this library will provide the optimal implementations for any
given architecture but maintain a consistent interface and allow it to be easily extended.

Hashes
------

You must #include the appropriate header for the hash family, for example::

   #include <Crypto/Sha1.h>

All hash operations are then performed via the ``Crypto::Sha1`` class.

Here's a basic example showing how to calculate a SHA1 hash on a C string::

   #include <Crypto/Sha1.h>

   void sha1Test(const char* buffer)
   {
      // Returns a Crypto::Sha1::Hash object
      auto hash = Crypto::Sha1().calculate(buffer, strlen(buffer));
      Serial.print("SHA1: ");
      Serial.println(Crypto::toString(hash));
   }

If your data has multiple chunks, use the longer form::

   #include <Crypto/Sha2.h>

   void sha256Test(const String& s1, const String& s2)
   {
      Crypto::Sha256 ctx;
      ctx.update(s1);
      ctx.update(s2);
      Serial.print("SHA256: ");
      Serial.println(Crypto::toString(ctx.getHash()));
   }

   sha256Test(F("This is some text to be hashed"), F("Hello"));

Some hashes have additional optional parameters, for example::

   #include <Crypto/Blake2s.h>
   
   void blake2sTest(const String& key, const String& content)
   {
      Crypto::Blake2s256 ctx(key);
      ctx.update(content);
      Serial.print("BLAKE2S-256: ");
      Serial.println(Crypto::toString(ctx.getHash()));
   }


HMAC
----

The `HMAC <https://en.wikipedia.org/wiki/HMAC>`__ algorithm is commonly used for verifying both
the integrity and authenticity of a message.
It can be used with any defined hash, commonly MD5 or SHA1.

For example, an MD5 HMAC (as used with `CRAM-MD5 <https://en.wikipedia.org/wiki/CRAM-MD5>`__)
may be done like this::

   #include <Crypto/Md5.h>

   void printHmacMd5(const String& key, const String& data)
   {
      auto hash = Crypto::HmacMd5(key).calculate(data);
      Serial.print("HMAC.MD5 = ");
      Serial.println(Crypto::toString(hash));
   }


'C' API
-------

The library also defines a standard 'C' api so it can be used from within existing code,
such as :component-esp8266:`axtls-28266` and :component-esp8266:`bearssl-esp8266`.
These definitions may be found in ``Crypto/HashApi``.


.. toctree::

   api

