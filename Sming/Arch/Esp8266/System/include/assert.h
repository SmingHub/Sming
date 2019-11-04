#include_next <assert.h>

#ifndef NDEBUG
#undef assert
#define assert(__e) ((__e) ? (void)0 : __assert_func(__FILE__, __LINE__, __ASSERT_FUNC, _F(#__e)))

#include <FakePgmSpace.h>

#endif
