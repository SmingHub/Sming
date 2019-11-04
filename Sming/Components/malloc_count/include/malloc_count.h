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
/**
 *  @brief Get the currently allocated amount of memory
 */
size_t getCurrent(void);

/**
 *  @brief Get the peak memory allocation
 */
size_t getPeak(void);

/**
 * @brief Reset the peak memory allocation to current
 */
void resetPeak(void);

/**
 * @brief Get the total number of allocations
 */
size_t getAllocCount(void);

/**
 * @brief Set an allocation limit
 * @param maxBytes Specify 0 for no limit
 */
void setAllocLimit(size_t maxBytes);

/**
 * @brief Callback function type
 * @param current Current allocated bytes
 */
typedef std::function<void(size_t current)> MallocCountCallback;

/**
 * @brief Set a callback function that is invoked on each change of the current allocation
 * @note The callback function must not use malloc()/realloc()/free()
 * or it will go into an endless recursive loop!
 */
void setCallback(MallocCountCallback callback);

/**
 * @brief Enable/disable logging
 */
void enableLogging(bool enable);

/**
 * @brief Set minimum allocation size for log output (when enabled)
 */
void setLogThreshold(size_t threshold);

}; // namespace MallocCount
