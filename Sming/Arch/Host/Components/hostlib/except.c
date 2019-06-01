/**
 * except.c
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

#include "except.h"
#include <signal.h>
#include <stdlib.h>
#include "hostapi.h"

static void signal_handler(int sig)
{
	host_puts("\r\n");
	switch(sig) {
	case SIGABRT:
		hostmsg("SIGABRT - usually caused by an abort() or assert()");
		break;
	case SIGFPE:
		hostmsg("SIGFPE - arithmetic exception such as divide by zero");
		break;
	case SIGILL:
		hostmsg("SIGILL - illegal instruction");
		break;
	case SIGINT:
		hostmsg("SIGINT - Ctrl+C pressed");
		host_exit(0);
		return;
	case SIGSEGV:
		hostmsg("SIGSEGV - segmentation fault");
		break;
	case SIGTERM:
		hostmsg("SIGTERM - a termination request was sent to the program");
		host_exit(1);
		return;
	default:
		hostmsg("Signal #%i", sig);
		break;
	}

	host_puts("\r\n");
	_Exit(1);
}

void trap_exceptions()
{
	signal(SIGABRT, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGTERM, signal_handler);
}
