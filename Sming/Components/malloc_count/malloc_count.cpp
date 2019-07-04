/******************************************************************************
 * malloc_count.c
 *
 * malloc() allocation counter based on http://ozlabs.org/~jk/code/ and other
 * code preparing LD_PRELOAD shared objects.
 *
 ******************************************************************************
 * Copyright (C) 2013-2014 Timo Bingmann <tb@panthema.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *****************************************************************************/

/*
 * @author July 2019 mikee47 <mike@sillyhouse.net>
 *
 * 	The original code uses dynamic loading to hook functions, which meant a separate
 * 	'init' heap was required to track initial usage before this took effect.
 * 	This has been changed to use function wrappers, so init heap no longer required and
 * 	code can be simplified.
 *
 *	Added user functions:
 *
 *		malloc_enable_logging
 *		malloc_set_log_threshold
 *
 */

#include "include/malloc_count.h"
#include <debug_progmem.h>
#include <esp_attr.h>

#ifdef ARCH_HOST
#define F_MALLOC malloc
#define F_CALLOC calloc
#define F_REALLOC realloc
#define F_FREE free
#else
#define F_MALLOC pvPortMalloc
#define F_CALLOC pvPortCalloc
#define F_REALLOC pvPortRealloc
#define F_FREE vPortFree
#endif

#define CONCAT(a, b) a##b
#define REAL(f) CONCAT(__real_, f)
#define WRAP(f) CONCAT(__wrap_, f)

extern "C" {
void* REAL(F_MALLOC)(size_t size);
void REAL(F_FREE)(void* ptr);
void* REAL(F_CALLOC)(size_t nmemb, size_t size);
void* REAL(F_REALLOC)(void* ptr, size_t size);
};

namespace MallocCount
{
/* user-defined options for output malloc()/free() operations to stderr */

static bool logEnabled = false;
static size_t logThreshold = 256;

void enableLogging(bool enable)
{
	logEnabled = enable;
}

void setLogThreshold(size_t threshold)
{
	logThreshold = threshold;
}

/* to each allocation additional data is added for bookkeeping. due to
 * alignment requirements, we can optionally add more than just one integer. */
static const size_t alignment = 16; /* bytes (>= 2*sizeof(size_t)) */

/* a sentinel value prefixed to each allocation */
static const size_t sentinel = 0xDEADC0DE;

/* Macro to get pointer to sentinel */
#define GET_SENTINEL(ptr) (size_t*)((char*)ptr - sizeof(size_t))

/* output */
#define PPREFIX "MC## "

#define log(fmt, ...)                                                                                                  \
	if(logEnabled) {                                                                                                   \
		debug_i(PPREFIX fmt, ##__VA_ARGS__);                                                                           \
	}

/*****************************************/
/* run-time memory allocation statistics */
/*****************************************/

static struct {
	size_t peak;	// Peak memory allocated
	size_t current; // Current memory allocated
	size_t total;   // Cumulative memory allocated
	size_t count;   // Number of allocations called
} stats;

static MallocCountCallback userCallback = nullptr;

/* add allocation to statistics */
static void inc_count(size_t inc)
{
	stats.current += inc;
	stats.total += inc;
	if(stats.current > stats.peak) {
		stats.peak = stats.current;
	}
	++stats.count;

	if(userCallback) {
		userCallback(stats.current);
	}
}

/* decrement allocation to statistics */
static void dec_count(size_t dec)
{
	stats.current -= dec;
	if(userCallback) {
		userCallback(stats.current);
	}
}

/* user function to return the currently allocated amount of memory */
size_t getCurrent()
{
	return stats.current;
}

/* user function to return the peak allocation */
size_t getPeak()
{
	return stats.peak;
}

/* user function to reset the peak allocation to current */
extern void resetPeak()
{
	stats.peak = stats.current;
}

/* user function to return total number of allocations */
extern size_t getAllocCount()
{
	return stats.count;
}

/* user function to supply a memory profile callback */
extern void setCallback(MallocCountCallback callback)
{
	userCallback = callback;
}

/****************************************************/
/* exported symbols that overlay the libc functions */
/****************************************************/

/* exported malloc symbol that overrides loading from libc */
extern "C" void* WRAP(F_MALLOC)(size_t size)
{
	if(size == 0) {
		return nullptr;
	}

	/* call read malloc procedure in libc */
	void* ret = REAL(F_MALLOC)(alignment + size);

	/* prepend allocation size and check sentinel */
	*(size_t*)ret = size;
	ret = (char*)ret + alignment;
	*GET_SENTINEL(ret) = sentinel;

	inc_count(size);
	if(size >= logThreshold) {
		log("malloc(%u) = %p (cur %u)", size, ret, stats.current);
	}

	return ret;
}

#ifndef ARCH_HOST
extern "C" void* WRAP(pvPortZalloc)(size_t size)
{
	auto ptr = WRAP(F_MALLOC)(size);
	if(ptr != nullptr) {
		memset(ptr, 0, size);
	}
	return ptr;
}

extern "C" void* WRAP(pvPortZallocIram)(size_t size, const char* file, int line)
	__attribute__((alias("__wrap_pvPortZalloc")));
#endif // ARCH_HOST

/* exported free symbol that overrides loading from libc */
extern "C" void WRAP(F_FREE)(void* ptr)
{
	// free(nullptr) is no operation
	if(ptr == nullptr) {
		return;
	}

	size_t* p_sentinel = GET_SENTINEL(ptr);
	if(*p_sentinel != sentinel) {
		log("free(%p) has no sentinel !!! memory corruption?", ptr);
		// ... or memory not allocated by our malloc()
	} else {
		*p_sentinel = 0; // Clear sentinel to avoid false-positives
		ptr = (char*)ptr - alignment;

		size_t size = *(size_t*)ptr;
		dec_count(size);

		if(size >= logThreshold) {
			log("free(%p) -> %u (cur %u)", ptr, size, stats.current);
		}
	}

	REAL(F_FREE)(ptr);
}

/* exported calloc() symbol that overrides loading from libc, implemented using our malloc */
extern "C" void* WRAP(F_CALLOC)(size_t nmemb, size_t size)
{
	size *= nmemb;
	if(size == 0) {
		return nullptr;
	}

	void* ret = malloc(size);
	memset(ret, 0, size);
	return ret;
}

/* exported realloc() symbol that overrides loading from libc */
extern "C" void* WRAP(F_REALLOC)(void* ptr, size_t size)
{
	// special case size == 0 -> free()
	if(size == 0) {
		WRAP(F_FREE)(ptr);
		return nullptr;
	}

	// special case ptr == 0 -> malloc()
	if(ptr == nullptr) {
		return WRAP(F_MALLOC)(size);
	}

	if(*GET_SENTINEL(ptr) != sentinel) {
		log("free(%p) has no sentinel !!! memory corruption?", ptr);
		// ... or memory not allocated by our malloc()
		return REAL(F_REALLOC)(ptr, size);
	}

	ptr = (char*)ptr - alignment;

	size_t oldsize = *(size_t*)ptr;

	dec_count(oldsize);
	inc_count(size);

	void* newptr = REAL(F_REALLOC)(ptr, alignment + size);

	if(size >= logThreshold) {
		if(newptr == ptr) {
			log("realloc(%u -> %u) = %p (cur %u)", oldsize, size, newptr, stats.current);
		} else {
			log("realloc(%u -> %u) = %p -> %p (cur %u)", oldsize, size, ptr, newptr, stats.current);
		}
	}

	*(size_t*)newptr = size;

	return (char*)newptr + alignment;
}

static __attribute__((destructor)) void finish()
{
	log("exiting, total: %u, peak: %u, current: %u", stats.total, stats.peak, stats.current);
}

}; // namespace MallocCount
