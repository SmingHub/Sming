#pragma once

#include_next <time.h>

#ifdef __WIN32

#ifdef __cplusplus
extern "C" {
#endif

struct tm* gmtime_r(const time_t*, struct tm*);

#ifdef __cplusplus
}
#endif

#endif
