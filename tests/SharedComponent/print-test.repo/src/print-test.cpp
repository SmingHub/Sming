#include "print-test.h"

#include <SmingCore.h>

/*
 * IMPORT_FSTR is useful if you want to include data from an external file.
 * It creates a global FlashString object so the name must be unique.
 * Here is one way to do that with a macro.
 */
#define DATA(name) print_test_##name

/*
 * The other problem is that the data needs to be associated with the component.
 * We do that here using the standard pre-defined __FILE__ string.
 * Our content is stored in files in a sub-directory called "print-test.cpp.data/".
 */
#define IMPORT_DATA(name) IMPORT_FSTR(DATA(name), __FILE__ ".data/" #name ".txt")

IMPORT_DATA(str1);
IMPORT_DATA(str2);

void print_test()
{
	m_puts(String(DATA(str1)).c_str());
	m_puts(String(DATA(str2)).c_str());
}
