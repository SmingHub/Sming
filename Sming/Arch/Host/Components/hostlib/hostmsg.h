/**
 * hostmsg.h - Print support for host output
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the Sming Framework Project
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with SHEM.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void host_printf(const char* fmt, ...);
void host_printfp(const char* fmt, const char* pretty_function, ...);
size_t host_nputs(const char* str, size_t length);
void host_puts(const char* str);

#ifdef __cplusplus
#define hostmsg(fmt, ...) host_printfp(fmt "\n", __PRETTY_FUNCTION__, ##__VA_ARGS__)
#else
#define hostmsg(fmt, ...) host_printfp(fmt "\n", __func__, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif
