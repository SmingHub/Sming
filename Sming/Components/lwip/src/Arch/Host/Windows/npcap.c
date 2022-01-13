/**
 * npcap.c
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

#include <pcap.h>

#include "npcap.h"

#undef PCAP_API
#define PCAP_API

// Casting of void* to function pointer not permitted by Ansi C
#pragma GCC diagnostic ignored "-Wpedantic"

WINAPI BOOL SetDllDirectoryA(LPCSTR lpPathName);

/*
 * Define a table for all the functions we need to bind. We use this to create for each function:
 *
 *		Function pointer - bound address of imported function
 *		Function typedef - so we can cast bound pointer for calling
 *		Function stub - passes call onto actual function
 *
 * If we get any of this wrong the compiler should complain as it'll check against prototypes in pcap.h.
 *
 * name, rettype, errval, argtypes, args
 *
 * 		name		Name of the function
 * 		rettype		Return type
 * 		errval		Value returned if function wasn't bound
 * 		argtypes	Parameter types for declaring the function
 * 		args		Parameter names, used for invoking the function
 */
#define PCAP_FUNCTIONS(XX)                                                                                             \
	XX(pcap_lib_version, const char*, NULL, (void), ())                                                                \
	XX(pcap_close, void, 0, (pcap_t * a), (a))                                                                         \
	XX(pcap_dispatch, int, PCAP_ERROR, (pcap_t * a, int b, pcap_handler c, u_char* d), (a, b, c, d))                   \
	XX(pcap_sendpacket, int, PCAP_ERROR, (pcap_t * a, const u_char* b, int c), (a, b, c))                              \
	XX(pcap_findalldevs, int, PCAP_ERROR, (pcap_if_t * *a, char* b), (a, b))                                           \
	XX(pcap_freealldevs, void, 0, (pcap_if_t * a), (a))                                                                \
	XX(pcap_open_live, pcap_t*, NULL, (const char* a, int b, int c, int d, char* e), (a, b, c, d, e))                  \
	XX(pcap_breakloop, void, 0, (pcap_t * a), (a))

#define XX(name, rettype, errval, argtypes, args) static void* pf_##name;
PCAP_FUNCTIONS(XX)
#undef XX

#define XX(name, rettype, errval, argtypes, args) typedef rettype(*name##_t) argtypes;
PCAP_FUNCTIONS(XX)
#undef XX

#define XX(name, rettype, errval, argtypes, args)                                                                      \
	rettype name argtypes                                                                                              \
	{                                                                                                                  \
		if(pf_##name == NULL) {                                                                                        \
			return (rettype)errval;                                                                                    \
		} else {                                                                                                       \
			return ((name##_t)pf_##name)args;                                                                          \
		}                                                                                                              \
	}
PCAP_FUNCTIONS(XX)
#undef XX

static HMODULE wpcap_module;
static void* bind_func(const char* name)
{
	void* pf = GetProcAddress(wpcap_module, name);
	if(pf == NULL) {
		fprintf(stderr, "Failed to bind '%s'", name);
	}
	return pf;
}

bool npcap_init(void)
{
	const char* NPCAP_DIR = "npcap";
	const char* WPCAP_DLL = "wpcap.dll";

	char path[MAX_PATH];
	GetSystemDirectory(path, MAX_PATH);
	strcat(path, "\\");
	strcat(path, NPCAP_DIR);
	SetDllDirectoryA(path);
	wpcap_module = LoadLibrary(WPCAP_DLL);
	if(wpcap_module == 0) {
		fprintf(stderr,
				"\n"
				"  ERROR! Failed to load 'wpcap.dll'\n"
				"  Have you installed the NPCAP library? See https://nmap.org/npcap/\n"
				"  Alternatively install Wireshark 3.0.2 or later. See https://www.wireshark.org/download.html\n"
				"\n");
		return false;
	}

#define XX(name, rettype, retval, argtypes, args)                                                                      \
	if((pf_##name = bind_func(#name)) == NULL) {                                                                       \
		return false;                                                                                                  \
	}
	PCAP_FUNCTIONS(XX)
#undef XX

	fprintf(stderr, "** %s\n", pcap_lib_version());

	return true;
}
