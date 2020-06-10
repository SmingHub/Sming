#include_next <assert.h>

// newlib 2.2 incorporates flash wrapper for assert
#ifndef __NEWLIB__

#ifndef NDEBUG
#undef assert
#define assert(__e) ((__e) ? (void)0 : __assert_func(__FILE__, __LINE__, __ASSERT_FUNC, _F(#__e)))

#include <FakePgmSpace.h>

#endif // NDEBUG

#endif // __NEWLIB__
