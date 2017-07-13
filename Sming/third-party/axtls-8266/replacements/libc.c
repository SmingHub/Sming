/*
 libc_replacements.c - replaces libc functions with functions
 from Espressif SDK
 Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 Modified 03 April 2015 by Markus Sattler
 */

#include <stdint.h>
#include "espinc/c_types_compatible.h"
#include <stdarg.h>

extern int ets_putc(int);
extern int ets_vprintf(int (*print_function)(int), const char * format, va_list arg);

int vprintf(const char * format, va_list arg) {
    return ets_vprintf(ets_putc, format, arg);
}
