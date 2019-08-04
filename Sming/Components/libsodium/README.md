# libsodium

From its documentation: *Sodium is a modern, easy-to-use software library for encryption, decryption, signatures, password hashing and more.*

This component integrates [libsodium](https://github.com/jedisct1/libsodium) v1.0.18 into Sming. To use it, simply add `COMPONENT_DEPENDS += libsodium` to your application's component.mk and `#include <sodium.h>` (or one of the more specific headers in sodium/*).

For further information, see libsodiums [documentation](https://libsodium.gitbook.io/doc/).

To build the library, Sming's standard component build process is used in favor of libsodium's original autotools based build process, which is not compatible with the xtensa-lx106-elf architecture. The list of source files, as well as compiler definitions, are hard-coded in component.mk according to the outcomes of (a hacked version of) the configure script.  
All optimizations leveraging x86/ARM architecture-specific assembly intructions are disabled and **only C reference implementations are used** instead. This is true even when compiling for the "Host" architecture.
As a side effect, **there is no need to invoke `sodium_init()` on application startup** (which would otherwise detect available CPU features and select optimal implementations accordingly). 

By default, the `randombytes_...` family of  functions is hooked up to the ESP8266 hardware random number generator. However, due to the lack of documentation, it is unclear if this generator provides sufficiently high quality random numbers for cryptographic purposes. Alternatively, a custom random number generator may be installed via `randombytes_set_implementation()`.



