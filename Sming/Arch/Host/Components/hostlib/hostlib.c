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

#include "hostlib.h"
#include <time.h>
#include <string.h>

int msleep(unsigned ms)
{
	struct timespec req, rem;
	req.tv_sec = ms / 1000;
	req.tv_nsec = (ms % 1000) * 1000000;
	return nanosleep(&req, &rem);
}

size_t getHostAppDir(char* path, size_t bufSize)
{
	if(path == NULL ||bufSize == 0) {
		return 0;
	}

	size_t len;
	char sep;
#ifdef __WIN32
	len = GetModuleFileName(NULL, path, bufSize);
	sep = '\\';
#else
	len = readlink("/proc/self/exe", path, bufSize - 1);
	sep = '/';
#endif
	path[len] = '\0';
	char* p = strrchr(path, sep);
	if(p != NULL) {
		p[1] = '\0';
		len = 1 + p - path;
	}
	return len;
}
