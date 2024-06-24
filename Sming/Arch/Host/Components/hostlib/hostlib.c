/**
 * hostlib.c
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

#include "include/hostlib/hostlib.h"
#include <time.h>
#include <string.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

int msleep(unsigned ms)
{
	struct timespec req, rem;
	req.tv_sec = ms / 1000;
	req.tv_nsec = (ms % 1000) * 1000000;
	return nanosleep(&req, &rem);
}

size_t getHostAppDir(char* path, size_t bufSize)
{
	if(path == NULL || bufSize == 0) {
		return 0;
	}

#ifdef __WIN32
	size_t len = GetModuleFileName(NULL, path, bufSize);
	char sep = '\\';
#elif defined(__APPLE__)
	uint32_t size = bufSize;
	size_t len = _NSGetExecutablePath(path, &size) ? 0 : size;
	char sep = '/';
#else
	size_t len = readlink("/proc/self/exe", path, bufSize - 1);
	char sep = '/';
#endif
	path[len] = '\0';
	char* p = strrchr(path, sep);
	if(p != NULL) {
		p[1] = '\0';
		len = 1 + p - path;
	}
	return len;
}
