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

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/reent.h>
#include <esp_attr.h>
#include <m_printf.h>

int _open_r(struct _reent* unused, const char* ptr, int mode)
{
	(void)unused;
	(void)ptr;
	(void)mode;
	return 0;
}

int _close_r(struct _reent* unused, int file)
{
	(void)unused;
	(void)file;
	return 0;
}

int _fstat_r(struct _reent* unused, int file, struct stat* st)
{
	(void)unused;
	(void)file;
	st->st_mode = S_IFCHR;
	return 0;
}

int _lseek_r(struct _reent* unused, int file, int ptr, int dir)
{
	(void)unused;
	(void)file;
	(void)ptr;
	(void)dir;
	return 0;
}

int _read_r(struct _reent* unused, int file, char* ptr, int len)
{
	(void)unused;
	(void)file;
	(void)ptr;
	(void)len;
	return 0;
}

int _write_r(struct _reent* r, int file, char* ptr, int len)
{
	(void)r;
	if(file == STDOUT_FILENO) {
		return m_nputs(ptr, len);
	}
	return 0;
}

int _isatty_r(struct _reent* r, int fd)
{
	(void)r;
	(void)fd;
	return 0;
}
