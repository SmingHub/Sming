/**
 * hostmsg.c
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "hostmsg.h"

/*
 * e.g. from "void a::sub(int)" we just want "a::sub"
 */
static const char* get_method_name(const char* pretty_function, size_t* length)
{
	const char* pbrace = strchr(pretty_function, '(');
	if(pbrace == NULL) {
		*length = strlen(pretty_function);
		return pretty_function;
	}
	const char* pcls = pbrace;
	while(pcls >= pretty_function && *pcls != ' ') {
		--pcls;
	}
	++pcls;
	*length = pbrace - pcls;
	return pcls;
}

void host_printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

void host_printfp(const char* fmt, const char* pretty_function, ...)
{
	size_t len;
	const char* name = get_method_name(pretty_function, &len);

	va_list args;
	va_start(args, pretty_function);
	char buffer[1024];
	memcpy(buffer, name, len);
	buffer[len++] = ':';
	buffer[len++] = ' ';
	vsnprintf(&buffer[len], sizeof(buffer) - len, fmt, args);
	va_end(args);

	host_puts(buffer);
}

void host_puts(const char* str)
{
	fputs(str, stderr);
}
