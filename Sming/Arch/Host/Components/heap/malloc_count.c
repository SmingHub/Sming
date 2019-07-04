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

#include "malloc_count.h"
#include <string.h>
#include <hostlib/hostmsg.h>

extern void* __real_malloc(size_t size);
extern void __real_free(void* ptr);
extern void* __real_calloc(size_t nmemb, size_t size);
extern void* __real_realloc(void* ptr, size_t size);

/* user-defined options for output malloc()/free() operations to stderr */

static bool log_operations = false;
static size_t log_operations_threshold = 256;

void malloc_enable_logging(bool enable)
{
	log_operations = enable;
}

void malloc_set_log_threshold(size_t threshold)
{
	log_operations_threshold = threshold;
}


/* to each allocation additional data is added for bookkeeping. due to
 * alignment requirements, we can optionally add more than just one integer. */
static const size_t alignment = 16; /* bytes (>= 2*sizeof(size_t)) */

/* a sentinel value prefixed to each allocation */
static const size_t sentinel = 0xDEADC0DE;

/* output */
#define PPREFIX "malloc_count ### "

/*****************************************/
/* run-time memory allocation statistics */
/*****************************************/

static long long peak = 0, curr = 0, total = 0, num_allocs = 0;

static malloc_count_callback_type callback = NULL;
static void* callback_cookie = NULL;

/* add allocation to statistics */
static void inc_count(size_t inc)
{
	if((curr += inc) > peak) {
		peak = curr;
	}
	total += inc;
	if(callback) {
		callback(callback_cookie, curr);
	}
	++num_allocs;
}

/* decrement allocation to statistics */
static void dec_count(size_t dec)
{
	curr -= dec;
	if(callback) {
		callback(callback_cookie, curr);
	}
}

/* user function to return the currently allocated amount of memory */
extern size_t malloc_count_current(void)
{
	return curr;
}

/* user function to return the peak allocation */
extern size_t malloc_count_peak(void)
{
	return peak;
}

/* user function to reset the peak allocation to current */
extern void malloc_count_reset_peak(void)
{
	peak = curr;
}

/* user function to return total number of allocations */
extern size_t malloc_count_num_allocs(void)
{
	return num_allocs;
}

/* user function which prints current and peak allocation to stderr */
extern void malloc_count_print_status(void)
{
	host_printf(PPREFIX "current %'lld, peak %'lld\n", curr, peak);
}

/* user function to supply a memory profile callback */
void malloc_count_set_callback(malloc_count_callback_type cb, void* cookie)
{
	callback = cb;
	callback_cookie = cookie;
}

/****************************************************/
/* exported symbols that overlay the libc functions */
/****************************************************/

/* exported malloc symbol that overrides loading from libc */
void* __wrap_malloc(size_t size)
{
	void* ret;

	if(size == 0) {
		return NULL;
	}

	/* call read malloc procedure in libc */
	ret = __real_malloc(alignment + size);

	inc_count(size);
	if(log_operations && size >= log_operations_threshold) {
		host_printf(PPREFIX "malloc(%'lld) = %p   (current %'lld)\n", (long long)size, (char*)ret + alignment, curr);
	}

	/* prepend allocation size and check sentinel */
	*(size_t*)ret = size;
	*(size_t*)((char*)ret + alignment - sizeof(size_t)) = sentinel;

	return (char*)ret + alignment;
}

/* exported free symbol that overrides loading from libc */
void __wrap_free(void* ptr)
{
	size_t size;

	if(!ptr) {
		return; /* free(NULL) is no operation */
	}

	ptr = (char*)ptr - alignment;

	if(*(size_t*)((char*)ptr + alignment - sizeof(size_t)) != sentinel) {
		host_printf(PPREFIX "free(%p) has no sentinel !!! memory corruption?\n", ptr);
	}

	size = *(size_t*)ptr;
	dec_count(size);

	if(log_operations && size >= log_operations_threshold) {
		host_printf(PPREFIX "free(%p) -> %'lld   (current %'lld)\n", ptr, (long long)size, curr);
	}

	__real_free(ptr);
}

/* exported calloc() symbol that overrides loading from libc, implemented using
 * our malloc */
void* __wrap_calloc(size_t nmemb, size_t size)
{
	size *= nmemb;
	if(size == 0) {
		return NULL;
	}

	void* ret = malloc(size);
	memset(ret, 0, size);
	return ret;
}

/* exported realloc() symbol that overrides loading from libc */
void* __wrap_realloc(void* ptr, size_t size)
{
	if(size == 0) { /* special case size == 0 -> free() */
		free(ptr);
		return NULL;
	}

	if(ptr == NULL) { /* special case ptr == 0 -> malloc() */
		return malloc(size);
	}

	ptr = (char*)ptr - alignment;

	if(*(size_t*)((char*)ptr + alignment - sizeof(size_t)) != sentinel) {
		host_printf(PPREFIX "free(%p) has no sentinel !!! memory corruption?\n", ptr);
	}

	size_t oldsize = *(size_t*)ptr;

	dec_count(oldsize);
	inc_count(size);

	void* newptr = __real_realloc(ptr, alignment + size);

	if(log_operations && size >= log_operations_threshold) {
		if(newptr == ptr)
			host_printf(PPREFIX "realloc(%'lld -> %'lld) = %p   (current %'lld)\n", (long long)oldsize, (long long)size,
						newptr, curr);
		else
			host_printf(PPREFIX "realloc(%'lld -> %'lld) = %p -> %p   (current %'lld)\n", (long long)oldsize,
						(long long)size, ptr, newptr, curr);
	}

	*(size_t*)newptr = size;

	return (char*)newptr + alignment;
}

static __attribute__((destructor)) void finish(void)
{
	host_printf(PPREFIX "exiting, total: %'lld, peak: %'lld, current: %'lld\n", total, peak, curr);
}
