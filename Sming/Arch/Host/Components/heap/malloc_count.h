/******************************************************************************
 * malloc_count.h
 *
 * Header containing prototypes of user-callable functions to retrieve run-time
 * information about malloc()/free() allocation.
 *
 ******************************************************************************
 * Copyright (C) 2013 Timo Bingmann <tb@panthema.net>
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

#pragma once

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" { /* for inclusion from C++ */
#endif

/* returns the currently allocated amount of memory */
extern size_t malloc_count_current(void);

/* returns the current peak memory allocation */
extern size_t malloc_count_peak(void);

/* resets the peak memory allocation to current */
extern void malloc_count_reset_peak(void);

/* returns the total number of allocations */
extern size_t malloc_count_num_allocs(void);

/* typedef of callback function */
typedef void (*malloc_count_callback_type)(void* cookie, size_t current);

/* supply malloc_count with a callback function that is invoked on each change
 * of the current allocation. The callback function must not use
 * malloc()/realloc()/free() or it will go into an endless recursive loop! */
extern void malloc_count_set_callback(malloc_count_callback_type cb, void* cookie);

/* user function which prints current and peak allocation to stderr */
extern void malloc_count_print_status(void);


extern void malloc_enable_logging(bool enable);
extern void malloc_set_log_threshold(size_t threshold);


#ifdef __cplusplus
} /* extern "C" */
#endif
