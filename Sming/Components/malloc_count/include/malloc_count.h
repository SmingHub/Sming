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
#include <functional>

namespace MallocCount
{
/* returns the currently allocated amount of memory */
size_t getCurrent(void);

/* returns the current peak memory allocation */
size_t getPeak(void);

/* resets the peak memory allocation to current */
void resetPeak(void);

/* returns the total number of allocations */
size_t getAllocCount(void);

/* typedef of callback function */
typedef std::function<void(size_t current)> MallocCountCallback;

/* supply malloc_count with a callback function that is invoked on each change
 * of the current allocation. The callback function must not use malloc()/realloc()/free()
 * or it will go into an endless recursive loop! */
void setCallback(MallocCountCallback callback);

/* dynamically enable/disable logging */
void enableLogging(bool enable);

/* allocations less than this threshold are never logged */
void setLogThreshold(size_t threshold);

}; // namespace MallocCount
