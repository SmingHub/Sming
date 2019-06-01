/**
 * hostlib.h
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

// Required for sleep(), ftruncate(), probably others
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L

#ifdef __WIN32
// Prevent early inclusion of winsock.h
#include <winsock2.h>
#undef EVENT_MAX // Conflicts with definitions in esp_wifi_types.h
#endif

#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (int)(sizeof(x) / sizeof((x)[0]))
#endif

// Used for parameterised token pasting
#ifndef JOIN
#define JOIN_AGAIN(_a, _b) _a##_b
#define JOIN(_a, _b) JOIN_AGAIN(_a, _b)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int msleep(unsigned ms);

// Include trailing path separator
void getHostAppDir(char* path, size_t bufSize);

#ifdef __cplusplus
}
#endif
