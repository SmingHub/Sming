/******************************************************************************
 * malloc_count.cpp
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
 *
 */

#include "include/malloc_count.h"
#include <debug_progmem.h>
#include <esp_attr.h>

// Names for the actual implementations
#ifdef ARCH_ESP8266

#define F_MALLOC pvPortMalloc
#define F_CALLOC pvPortCalloc
#define F_REALLOC pvPortRealloc
#define F_FREE vPortFree

#else

#define F_MALLOC malloc
#define F_CALLOC calloc
#define F_REALLOC realloc
#define F_FREE free

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

namespace
{
/* user-defined options for output malloc()/free() operations to stderr */
bool logEnabled{false};
size_t logThreshold{256};

/* to each allocation additional data is added for bookkeeping. due to
 * alignment requirements, we can optionally add more than just one integer. */
constexpr size_t alignment{16}; /* bytes (>= 2*sizeof(size_t)) */

/* a sentinel value prefixed to each allocation */
constexpr size_t sentinel{0xDEADC0DE};

/* Add an offset to a pointer return that as a pointer cast to required type */
template <typename T = void*> T offsetPointer(void* ptr, intptr_t offset)
{
	return reinterpret_cast<T>(reinterpret_cast<intptr_t>(ptr) + offset);
}

/* Get pointer to sentinel */
size_t* getSentinel(void* ptr)
{
	return offsetPointer<size_t*>(ptr, -sizeof(size_t));
}

/* output */
#define PPREFIX "MC## "

#define log(fmt, ...)                                                                                                  \
	if(logEnabled) {                                                                                                   \
		debug_i(PPREFIX fmt, ##__VA_ARGS__);                                                                           \
	}

/*****************************************/
/* run-time memory allocation statistics */
/*****************************************/

struct Stats {
	size_t peak;	// Peak memory allocated
	size_t current; // Current memory allocated
	size_t total;   // Cumulative memory allocated
	size_t count;   // Number of allocations called
};

Stats stats{};

size_t allocationLimit{0};

MallocCount::Callback userCallback;

#ifdef ENABLE_MALLOC_COUNT

/* add allocation to statistics */
void inc_count(size_t inc)
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
void dec_count(size_t dec)
{
	stats.current -= dec;
	if(userCallback) {
		userCallback(stats.current);
	}
}

#endif // ENABLE_MALLOC_COUNT

} // namespace

namespace MallocCount
{
void enableLogging(bool enable)
{
	logEnabled = enable;
}

void setLogThreshold(size_t threshold)
{
	logThreshold = threshold;
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
void resetPeak()
{
	stats.peak = stats.current;
}

size_t getTotal()
{
	return stats.total;
}

void resetTotal()
{
	stats.total = 0;
}

/* user function to return total number of allocations */
size_t getAllocCount()
{
	return stats.count;
}

/* sets the maximum available memory */
void setAllocLimit(size_t maxBytes)
{
	allocationLimit = maxBytes;
}

/* user function to supply a memory profile callback */
void setCallback(Callback callback)
{
	userCallback = std::move(callback);
}

#ifdef ENABLE_MALLOC_COUNT

/****************************************************/
/* malloc_count function implementations             */
/****************************************************/

extern "C" void* mc_malloc(size_t size)
{
	if(size == 0) {
		return nullptr;
	}

	if(allocationLimit != 0 && stats.current + size > allocationLimit) {
		log("malloc(%u) -> exceeds maximum (current is %u bytes)", size, stats.current);
		return nullptr;
	}

	/* call read malloc procedure in libc */
	void* ret = REAL(F_MALLOC)(alignment + size);

	if(ret == nullptr) {
		log("malloc(%u) failed", size);
		return ret;
	}

	/* prepend allocation size and check sentinel */
	*static_cast<size_t*>(ret) = size;
	ret = offsetPointer(ret, alignment);
	*getSentinel(ret) = sentinel;

	inc_count(size);
	if(size >= logThreshold) {
		log("malloc(%u) = %p (cur %u)", size, ret, stats.current);
	}

	return ret;
}

extern "C" void* mc_zalloc(size_t size)
{
	auto ptr = mc_malloc(size);
	if(ptr != nullptr) {
		memset(ptr, 0, size);
	}
	return ptr;
}

extern "C" void mc_free(void* ptr)
{
	// free(nullptr) is no operation
	if(ptr == nullptr) {
		return;
	}

	size_t* p_sentinel = getSentinel(ptr);
	if(*p_sentinel != sentinel) {
		log("free(%p) has no sentinel !!! memory corruption?", ptr);
		// ... or memory not allocated by our malloc()
	} else {
		*p_sentinel = 0; // Clear sentinel to avoid false-positives
		ptr = offsetPointer(ptr, -alignment);

		size_t size = *static_cast<size_t*>(ptr);
		dec_count(size);

		if(size >= logThreshold) {
			log("free(%p) -> %u (cur %u)", offsetPointer(ptr, alignment), size, stats.current);
		}
	}

	REAL(F_FREE)(ptr);
}

extern "C" void* mc_calloc(size_t nmemb, size_t size)
{
	return mc_zalloc(nmemb * size);
}

extern "C" void* mc_realloc(void* ptr, size_t size)
{
	// special case size == 0 -> free()
	if(size == 0) {
		mc_free(ptr);
		return nullptr;
	}

	// special case ptr == 0 -> malloc()
	if(ptr == nullptr) {
		return mc_malloc(size);
	}

	if(*getSentinel(ptr) != sentinel) {
		log("free(%p) has no sentinel !!! memory corruption?", ptr);
		// ... or memory not allocated by our malloc()
		return REAL(F_REALLOC)(ptr, size);
	}

	// For testing purposes we'll assume the reallocation won't be in-place
	if(allocationLimit != 0 && stats.current + size > allocationLimit) {
		log("remalloc(%u) -> exceeds maximum (current is %u bytes)", size, stats.current);
		return nullptr;
	}

	ptr = offsetPointer(ptr, -alignment);

	size_t oldsize = *static_cast<size_t*>(ptr);

	void* newptr = REAL(F_REALLOC)(ptr, alignment + size);

	if(newptr == nullptr) {
		log("realloc(%u -> %u) failed", oldsize, size);
		return nullptr;
	}

	dec_count(oldsize);
	inc_count(size);

	if(size >= logThreshold) {
		if(newptr == ptr) {
			log("realloc(%u -> %u) = %p (cur %u)", oldsize, size, offsetPointer(newptr, alignment), stats.current);
		} else {
			log("realloc(%u -> %u) = %p -> %p (cur %u)", oldsize, size, offsetPointer(ptr, alignment),
				offsetPointer(newptr, alignment), stats.current);
		}
	}

	*static_cast<size_t*>(newptr) = size;

	return offsetPointer(newptr, alignment);
}

static __attribute__((destructor)) void finish()
{
	log("exiting, total: %u, peak: %u, current: %u", stats.total, stats.peak, stats.current);
}

#endif // ENABLE_MALLOC_COUNT

}; // namespace MallocCount

#ifdef ENABLE_MALLOC_COUNT

/****************************************************/
/* exported symbols that overlay the libc functions */
/****************************************************/

extern "C" void* WRAP(malloc)(size_t) __attribute__((alias("mc_malloc")));
extern "C" void* WRAP(calloc)(size_t, size_t) __attribute__((alias("mc_calloc")));
extern "C" void* WRAP(realloc)(void*, size_t) __attribute__((alias("mc_realloc")));
extern "C" void WRAP(free)(void*) __attribute__((alias("mc_free")));

using namespace MallocCount;

#ifdef ARCH_ESP8266

extern "C" void* WRAP(pvPortMalloc)(size_t) __attribute__((alias("mc_malloc")));
extern "C" void* WRAP(pvPortCalloc)(size_t, size_t) __attribute__((alias("mc_calloc")));
extern "C" void* WRAP(pvPortRealloc)(void*, size_t) __attribute__((alias("mc_realloc")));
extern "C" void* WRAP(pvPortZalloc)(size_t) __attribute__((alias("mc_zalloc")));
extern "C" void* WRAP(pvPortZallocIram)(size_t) __attribute__((alias("mc_zalloc")));
extern "C" void WRAP(vPortFree)(void*) __attribute__((alias("mc_free")));

#else

void* operator new(size_t size)
{
	return mc_malloc(size);
}

void* operator new(size_t size, const std::nothrow_t&) noexcept
{
	return mc_malloc(size);
}

void* operator new[](size_t size)
{
	return mc_malloc(size);
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept
{
	return mc_malloc(size);
}

void operator delete(void* ptr)
{
	mc_free(ptr);
}

void operator delete[](void* ptr)
{
	mc_free(ptr);
}

#if __cplusplus >= 201402L

void operator delete(void* ptr, size_t)
{
	mc_free(ptr);
}

void operator delete[](void* ptr, size_t)
{
	mc_free(ptr);
}

#endif

#endif // ESP8266

extern "C" char* WRAP(strdup)(const char* s)
{
	auto len = strlen(s) + 1;
	auto dup = static_cast<char*>(mc_malloc(len));
	memcpy(dup, s, len);
	return dup;
}

#endif // ENABLE_MALLOC_COUNT
