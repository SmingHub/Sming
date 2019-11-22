/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * esp_cplusplus.cpp
 *
 ****/

#include <esp_systemapi.h>
#include "include/esp_cplusplus.h"
#include <stdlib.h>

extern void (*__init_array_start)();
extern void (*__init_array_end)();

////////////////////////////////////////////////////////////////////////

// Just do it! :)
void cpp_core_initialize()
{
    void (**p)(void);
    for (p = &__init_array_start; p != &__init_array_end; ++p) {
            (*p)();
    }
}

extern "C" void __cxa_pure_virtual(void)
{
	SYSTEM_ERROR("Bad pure_virtual_call");
	abort();
}

extern "C" void __cxa_deleted_virtual(void)
{
	SYSTEM_ERROR("Bad deleted_virtual_call");
	abort();
}

namespace std {
    void WEAK_ATTR __throw_bad_function_call()
    {
        while(1);
    };
}
