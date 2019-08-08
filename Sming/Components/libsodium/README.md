
# libsodium

From its documentation: *Sodium is a modern, easy-to-use software library for encryption, decryption, signatures, password hashing and more.*

## Usage
This component integrates [libsodium](https://github.com/jedisct1/libsodium) v1.0.18 into Sming. To use it, simply add `COMPONENT_DEPENDS += libsodium` to your application's component.mk and `#include <sodium.h>` (or one of the more specific headers in sodium/*).

For further information, see libsodiums [documentation](https://libsodium.gitbook.io/doc/).

## Build Details
To build the library, Sming's standard component build process is used in favor of libsodium's original autotools based build process, which is not compatible with the xtensa-lx106-elf architecture. The list of source files, as well as compiler definitions, are hard-coded in component.mk according to the outcomes of (a hacked version of) the configure script.  
All optimizations leveraging x86/ARM architecture-specific assembly intructions are disabled and **only C reference implementations are used** instead. This is true even when compiling for the "Host" architecture.
As a side effect, **there is no need to invoke `sodium_init()` on application startup** (which would otherwise detect available CPU features and select optimal implementations accordingly). 

## Notes on Random Number Generation 
By default, the `randombytes_...` family of  functions is hooked up to the ESP8266 hardware random number generator via `os_[get_]random`, which is also available in the Host emulator. However, due to the lack of documentation, it is unclear if the hardware random number generator provides sufficiently high quality random numbers for cryptographic purposes. Some additional information can be found [here](https://raw.githubusercontent.com/pfalcon/esp8266-re-wiki-mirror/master/Random_Number_Generator.mw). Also note that the host emulator may not use a high-quality random number source and therefore should not be trusted with generating private keys and other sensitive data.
Alternatively, libsodium offers the possibility to install a custom random number generator implementation via `randombytes_set_implementation()`, which is fully controllable by the user.
