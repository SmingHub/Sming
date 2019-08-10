/**
 * threads.cpp
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

#include "threads.h"
#include <signal.h>

CMutex CThread::interrupt;

#ifdef __WIN32

static HANDLE mainThread;

#else

static pthread_t mainThread;
static CSemaphore mainThreadSemaphore;

static void signal_handler(int sig)
{
	if(sig == SIGUSR1) {
		mainThreadSemaphore.wait();
	}
}

#endif

void CThread::startup()
{
#ifdef __WIN32
	mainThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetCurrentThreadId());
#else
	mainThread = pthread_self();
	signal(SIGUSR1, signal_handler);
#endif
}

static void suspend_main_thread(bool suspend)
{
#ifdef __WIN32
	if(suspend) {
		SuspendThread(mainThread);
	} else {
		ResumeThread(mainThread);
	}
#else
	if(suspend) {
		pthread_kill(mainThread, SIGUSR1);
	} else {
		mainThreadSemaphore.post();
	}
#endif
}

void CThread::set_interrupt_level(unsigned new_level)
{
	if(new_level > 0) {
		interrupt.lock();
		suspend_main_thread(true);
	} else {
		suspend_main_thread(false);
		interrupt.unlock();
	}
}

void CThread::interrupt_lock()
{
	interrupt.lock();
}

void CThread::interrupt_unlock()
{
	interrupt.unlock();
}
