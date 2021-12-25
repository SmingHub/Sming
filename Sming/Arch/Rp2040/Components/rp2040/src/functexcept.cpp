// Function-Based Exception Support -*- C++ -*-

// Copyright (C) 2001-2020 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file bits/functexcept.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{exception}
 *
 *  This header provides support for -fno-exceptions.
 */

//
// ISO C++ 14882: 19.1  Exception classes
//

#include <sming_attr.h>

#define THROW_HANDLER(name, ...)                                                                                       \
	void WEAK_ATTR __throw_##name(__VA_ARGS__)                                                                         \
	{                                                                                                                  \
		while(1) {                                                                                                     \
		}                                                                                                              \
	}

namespace std
{
// Helper for exception objects in <except>
THROW_HANDLER(bad_exception, void)

// Helper for exception objects in <new>
THROW_HANDLER(bad_alloc, void)

// Helper for exception objects in <typeinfo>
THROW_HANDLER(bad_cast, void)

THROW_HANDLER(bad_typeid, void)

// Helpers for exception objects in <stdexcept>
THROW_HANDLER(logic_error, const char*)

THROW_HANDLER(domain_error, const char*)

THROW_HANDLER(invalid_argument, const char*)

THROW_HANDLER(length_error, const char*)

THROW_HANDLER(out_of_range, const char*)

THROW_HANDLER(out_of_range_fmt, const char*, ...)

THROW_HANDLER(runtime_error, const char*)

THROW_HANDLER(range_error, const char*)

THROW_HANDLER(overflow_error, const char*)

THROW_HANDLER(underflow_error, const char*)

// Helpers for exception objects in <ios>
THROW_HANDLER(ios_failure, const char*)

THROW_HANDLER(ios_failure, const char*, int)

// Helpers for exception objects in <system_error>
THROW_HANDLER(system_error, int)

// Helpers for exception objects in <future>
THROW_HANDLER(future_error, int)

// Helpers for exception objects in <functional>
THROW_HANDLER(bad_function_call)

} // namespace std
