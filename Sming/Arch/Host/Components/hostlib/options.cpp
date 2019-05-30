/**
 * options.cpp
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

#include "options.h"
#include "hostmsg.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

struct option_help_t {
	const char* brief;
	const char* argname;
	const char* arghelp;
	const char* description;
};

static const option long_options[] = {
#define XX(tag, has_arg, desc, argname, arghelp, examples) {#tag, has_arg},
	OPTION_MAP(XX)
#undef XX
		{nullptr},
};

static const option_help_t option_help[] = {
#define XX(tag, has_arg, desc, argname, arghelp, examples) {desc, argname, arghelp, examples},
	OPTION_MAP(XX)
#undef XX
};

void print_help()
{
	const unsigned COL = 23;
	auto help = option_help;
	auto opt = long_options;
	for(; opt->name; ++opt, ++help) {
		char s[256];
		strcpy(s, opt->name);
		if(help->argname) {
			if(opt->has_arg == optional_argument) {
				strcat(s, "[");
			}
			strcat(s, "=");
			strcat(s, help->argname);
			if(opt->has_arg == optional_argument) {
				strcat(s, "]");
			}
		}
		host_printf("%*s%s\n", -COL, s, help->brief);
		if(help->argname) {
			host_printf("%*s%s - %s\n", COL + 2, "", help->argname, help->arghelp);
		}
		auto ex = help->description;
		if(ex) {
			while(*ex) {
				host_printf("%*s%s\n", COL + 2, "", ex);
				ex += strlen(ex) + 1;
			}
		}
	}
}

option_tag_t get_option(int argc, char* argv[], const char*& arg)
{
	int option_index = 0;
	int c = getopt_long_only(argc, argv, "", long_options, &option_index);
	if(c == '?') {
		host_printf("Invalid command line. Try --help\n");
		exit(1);
	}
	if(c != 0) {
		return opt_none;
	}

	arg = optarg;
	return option_tag_t(option_index);
}
