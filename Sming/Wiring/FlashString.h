/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 * Defines the FlashString structure and associated macros for efficient flash memory string access.
 *
 ****/

/*
 * Note on storing strings in flash memory
 *
 * A string literal (e.g. "string") used in code gets emitted to the .rodata segment by the compiler.
 * That means it gets read into RAM at startup and remains there.
 * To avoid this, and reclaim the RAM, the data must be stored in a different segment. This is done
 * using the PROGMEM macro, defined in FakePgmSpace.h.
 *
 * Once in flash memory, string data must be read into RAM before it can be used. Accessing the flash
 * memory directly is awkard. If locations are not strictly accessed as 4-byte words the system will probably
 * crash; I say 'probably' because sometimes it just behaves weirdly if the RAM address isn't aligned.
 *
 * _FakePgmSpace_ provides the basic mechanisms for storing and reading flash strings, including
 * general-purpose string library functions.  These are well-documented Arduino-compatible routines.
 * Some additions have been made to Sming to cater for the ESP8266 use of these strings.
 *
 * 	F(string_literal) - loads a String object with the given text, which is allocated to flash.
 *		String s = F("test");
 *
 *	_F(string_literal) - Like F() except buffer is allocated on stack. Most useful where nul-terminated data is required.
 *		m_printf(_F("C-style string\n"));
 *
 *	DEFINE_PSTR(_name, _str) - declares a PSTR stored in flash. The variable (_name) points to flash
 *	memory so must be accessed using the appropriate xxx_P function.
 *
 *	LOAD_PSTR(_name, _flash_str) - loads pre-defined PSTR into buffer on stack
 *		DEFINE_PSTR_LOCAL(testFlash, "This is a test string\n"); // Function scope requires static allocation
 *		LOAD_PSTR(test, testFlash)
 *		m_printf(test);
 *
 *	PSTR_ARRAY(_name, _str) - creates and loads string into named stack buffer
 *	Ensures loaded string stays in scope, unlike _F()
 *		String testfunc() {
 *			PSTR_ARRAY(test, "This is the test string");
 *			m_printf(test);
 *			...
 *      	return test;
 *      }
 *
 *  IMPORT_FSTR(name, file) - binds a file into the firmware image as a FlashString object
 *  This needs to be used at file scope, example:
 *  	IMPORT_FSTR(myFlashData, PROJECT_DIR "/files/myFlashData.bin")
 *
 *  Note the use of PROJECT_DIR to locate the file using an absolute path.
 *
 *	Both DEFINE_PSTR and PSTR_ARRAY load a PSTR into a stack buffer, but using sizeof() on that buffer will return
 *	a larger value than the string itself because it's aligned. Calling sizeof() on the original flash data will
 *	get the right value. If it's a regular nul-terminated string then strlen_P() will get the length, although it's
 *	time-consuming.
 *
 *	FlashString
 *
 *		A 'counted string' defined as a C++ structure with a length field.
 *		Requires only a single flash read to get the length; no strlen_P, etc.
 *		Stores arbitrary data including nuls.
 *		A global instance can be shared amongst modules with no loss of efficiency.
 *		The data is word-aligned so reading is as fast as possible; there are no alignment issues to deal with.
 *		Structure-based, so can add methods to the structure instead of using macros.
 *		Strongly typed, so support can be added to other modules for implicit conversions, etc. (e.g. ArduinoJson)
 *		Works with updated String class to provide the simplest string/data management.
 *
 *		Macros are consistent with the PSTR types, but named 'FSTR'.
 *
 *  Notes on usage
 *
 *  	Best practice is usually to define constant data at the top of a module. Non-trivial strings are no different.
 *  	The mechanisms offered here provide a way to help do that.
 *
 */

#pragma once

#include "WString.h"

/** @brief Define a FlashString
 *  @param _name variable to identify the string
 *  @param _str content of the string
 *  @note the whole thing is word-aligned
 *  Example: DEFINE_FSTR(test, "This is a test\0Another test\0hello")
 *  The data includes the nul terminator but the length does not.
 */
#define DEFINE_FSTR(_name, _str)                                                                                       \
	DEFINE_FSTR_STRUCT(_##_name, _str);                                                                                \
	const FlashString& _name PROGMEM = _##_name.fstr;

/** @brief Define a FlashString for local (static) use
 *  @param _name variable to identify the string
 *  @param _str content of the string
 */
#define DEFINE_FSTR_LOCAL(_name, _str)                                                                                 \
	static DEFINE_FSTR_STRUCT(_##_name, _str);                                                                         \
	static const FlashString& _name PROGMEM = _##_name.fstr;

#define DEFINE_FSTR_STRUCT(_name, _str)                                                                                \
	constexpr struct {                                                                                                 \
		FlashString fstr;                                                                                              \
		char data[ALIGNUP(sizeof(_str))];                                                                              \
	} _name PROGMEM = {{sizeof(_str) - 1}, _str};

// Declare a global reference to a FlashString instance
#define DECLARE_FSTR(_name) extern const FlashString& _name;

// Get a pointer to the actual FlashString, used when creating tables
#define FSTR_PTR(_struct) &_##_struct.fstr

/** @brief declare a table of FlashStrings
 *  @param _name name of the table
 *  @note Declares a lookup table stored in flash memory. Example:
 *
 *  	DEFINE_FSTR(fstr1, "Test string #1");
 *  	DEFINE_FSTR(fstr2, "Test string #2");
 *
 *  	FSTR_TABLE(table) = {
 *  		FSTR_PTR(fstr1),
 *  		FSTR_PTR(fstr2),
 *  	};
 *
 *  Table entries may be accessed directly as they are word-aligned. Examples:
 *  	debugf("fstr1 = '%s'", String(*table[0]).c_str());
 *  	debugf("fstr2.length() = %u", table[1]->length());
 *
 */
#define FSTR_TABLE(_name) const FlashString* const _name[] PROGMEM

/*
 * Load a FlashString object into a named local (stack) buffer
 *
 * For example:
 *
 * 	DEFINE_FSTR(globalTest, "This is a testing string")
 * 	...
 * 	LOAD_FSTR(local, globalTest)
 * 	printf("%s, %u characters, buffer is %u bytes\n", local, globalTest.length, sizeof(local));
 */
#define LOAD_FSTR(_name, _fstr)                                                                                        \
	char _name[(_fstr).size()] __attribute__((aligned(4)));                                                            \
	memcpy_aligned(_name, (_fstr).data(), (_fstr).length());                                                           \
	_name[(_fstr).length()] = '\0';

/*
 * Define a flash string and load it into a named char[] buffer on the stack.
 * This allows sizeof(_name) to work as if the string were defined thus:
 *
 * 	char _name[] = "text";
 */
#define FSTR_ARRAY(_name, _str)                                                                                        \
	DEFINE_FSTR_LOCAL(_##_name, _str);                                                                                 \
	LOAD_FSTR(_name, _##_name)

/** @brief Define a FlashString containing data from an external file
 *  @param name Name to use for referencing the FlashString object in code
 *  @param file Path to the file to be included. This should be an absolute path.
 *  @note This provides a more efficient way to read constant (read-only) file data.
 *  The file content is bound into firmware image at link time.
 *  @note The FlashString object must be referenced or the linker won't emit it.
 *  @note Use the PROJECT_DIR to locate files in your project's source tree. For example:
 *  		IMPORT_FSTR(myFlashString, PROJECT_DIR "/files/my_flash_file.txt");
 */
#ifdef __WIN32
#define IMPORT_FSTR(name, file)                                                                                        \
	__asm__(".section .rodata\n"                                                                                       \
			".global _" #name "\n"                                                                                     \
			".def _" #name "; .scl 2; .type 32; .endef\n"                                                              \
			".align 4\n"                                                                                               \
			"_" #name ":\n"                                                                                            \
			".long _" #name "_end - _" #name " - 4\n"                                                                  \
			".incbin \"" file "\"\n"                                                                                   \
			"_" #name "_end:\n");                                                                                      \
	extern const __attribute__((aligned(4))) FlashString name;
#else
#ifdef ARCH_HOST
#define IROM_SECTION ".rodata"
#else
#define IROM_SECTION ".irom0.text"
#endif
#define IMPORT_FSTR(name, file)                                                                                        \
	__asm__(".section " IROM_SECTION "\n"                                                                              \
			".global " #name "\n"                                                                                      \
			".type " #name ", @object\n"                                                                               \
			".align 4\n" #name ":\n"                                                                                   \
			".long _" #name "_end - " #name " - 4\n"                                                                   \
			".incbin \"" file "\"\n"                                                                                   \
			"_" #name "_end:\n");                                                                                      \
	extern const __attribute__((aligned(4))) FlashString name;
#endif

/** @brief describes a counted string stored in flash memory
 *  @note because the string length is stored there is no need to call strlen_P before reading the
 *  content into RAM. Data is stored word-aligned so it can be read as efficiently as possible.
 */
struct FlashString {
	// Do NOT access these directly - use member functions
	uint32_t flashLength; ///< Only needs to be uint16_t but ensures data is aligned
	char flashData[];

	uint32_t length() const
	{
		return flashLength;
	}

	uint32_t size() const
	{
		return ALIGNUP(flashLength + 1);
	}

	flash_string_t data() const
	{
		return FPSTR(flashData);
	}

	/** @brief Check for equality with a C-string
	 *  @param cstr
	 *  @retval bool true if strings are identical
	 *  @note loads string into a stack buffer for the comparison, no heap required
	 */
	bool isEqual(const char* cstr) const
	{
		// Unlikely we'd want an empty flash string, but check anyway
		if(cstr == nullptr)
			return flashLength == 0;
		// Don't use strcmp as our data may contain nuls
		size_t cstrlen = strlen(cstr);
		if(cstrlen != flashLength)
			return false;
		LOAD_FSTR(buf, *this);
		return memcmp(buf, cstr, cstrlen) == 0;
	}

	/** @brief Check for equality with another FlashString
	 *  @param str
	 *  @retval bool true if strings are identical
	 */
	bool isEqual(const FlashString& str) const
	{
		if(flashLength != str.flashLength)
			return false;
		if(flashData == str.flashData)
			return true;
		return memcmp_aligned(flashData, str.flashData, flashLength) == 0;
	}

	bool isEqual(const String& str) const
	{
		return str.equals(*this);
	}

	bool operator==(const char* str) const
	{
		return isEqual(str);
	}

	bool operator==(const FlashString& str) const
	{
		return isEqual(str);
	}

	bool operator==(const String& str) const
	{
		return isEqual(str);
	}

	bool operator!=(const char* str) const
	{
		return !isEqual(str);
	}

	bool operator!=(const FlashString& str) const
	{
		return !isEqual(str);
	}

	bool operator!=(const String& str) const
	{
		return !isEqual(str);
	}
};
