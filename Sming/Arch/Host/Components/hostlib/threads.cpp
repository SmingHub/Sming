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
#include <cstring>
#include <cstdarg>
#include <signal.h>

CThread::List CThread::list;
unsigned CThread::interrupt_mask;
CBasicMutex* interrupt;

namespace
{
pthread_t mainThread;

#ifndef __WIN32

volatile bool mainThreadSignalled;
timer_t signalTimer;
int pauseSignal;
int resumeSignal;

void signal_handler(int sig)
{
	if(sig == pauseSignal) {
		mainThreadSignalled = true;
		/*
		 * A resumeSignal here results in deadlock as we're waiting for a signal which will never arrive.
		 * Possibly useful async-safe functions:
		 * 
		 * - sem_post()
		 * - alarm(secs) Delivers SIGALRM after a delay.
		 * - timer_settime() as for alarm() but with smaller interval
		 *
		 */
		struct timespec ts = {0, long(0.1e9)};
		struct itimerspec its = {ts, ts};
		timer_settime(signalTimer, 0, &its, nullptr);
		while(mainThreadSignalled) {
			pause();
		}
		its = {};
		timer_settime(signalTimer, 0, &its, nullptr);
	} else if(sig == resumeSignal) {
		mainThreadSignalled = false;
	} else if(sig == SIGALRM) {
	} else {
		assert(false);
	}
}

#endif

bool isMainThread()
{
	return pthread_equal(pthread_self(), mainThread);
}

void suspend_main_thread()
{
	assert(!isMainThread());

#ifdef __WIN32
	SuspendThread(pthread_getw32threadhandle_np(mainThread));
#else

	assert(!mainThreadSignalled);
	assert(pthread_kill(mainThread, pauseSignal) == 0);
	while(!mainThreadSignalled) {
		sched_yield();
	}
#endif
}

void resume_main_thread()
{
	assert(!isMainThread());

#ifdef __WIN32
	ResumeThread(pthread_getw32threadhandle_np(mainThread));
#else
	assert(mainThreadSignalled);
	assert(pthread_kill(mainThread, resumeSignal) == 0);
	while(mainThreadSignalled) {
		sched_yield();
	}
#endif
}

} // namespace

void CMutex::lock()
{
	interrupt->lock();
	CBasicMutex::lock();
}

void CMutex::unlock()
{
	CBasicMutex::unlock();
	interrupt->unlock();
}

void CThread::startup()
{
	mainThread = pthread_self();
	interrupt = new CBasicMutex;
#ifndef __WIN32
	pauseSignal = SIGRTMIN + 0;
	resumeSignal = SIGRTMIN + 1;
	signal(pauseSignal, signal_handler);
	signal(resumeSignal, signal_handler);
	signal(SIGALRM, signal_handler);
	timer_create(CLOCK_MONOTONIC, nullptr, &signalTimer);
#endif
}

CThread::CThread(const char* name, unsigned interrupt_level) : name(name), interrupt_level(interrupt_level)
{
	assert(interrupt_level > 0);
	interrupt->lock();
	list.add(this);
	interrupt->unlock();
}

CThread::~CThread()
{
	HOST_THREAD_DEBUG("Thread '%s' destroyed", name);
	interrupt->lock();
	list.remove(this);
	interrupt->unlock();
}

void CThread::interrupt_lock()
{
	assert(interrupt_mask == 0);
	interrupt->lock();
}

void CThread::interrupt_unlock()
{
	assert(interrupt_mask == 0);
	interrupt->unlock();
}

void CThread::suspend()
{
	assert(!isCurrent());
	suspendMutex.lock();
	++suspended;
	suspendMutex.unlock();
}

void CThread::resume()
{
	assert(!isCurrent());
	suspendMutex.lock();
	--suspended;
	if(suspended == 0) {
		pthread_cond_signal(&resumeCond);
	}
	suspendMutex.unlock();
}

void CThread::interrupt_begin()
{
	assert(isCurrent());
	assert(interrupt_level > interrupt_mask);

	// Are we suspended by another thread?
	suspendMutex.lock();
	while(suspended != 0) {
		suspendMutex.wait(resumeCond);
	}
	suspendMutex.unlock();

	interrupt->lock();

	if(interrupt_mask == 0) {
		suspend_main_thread();
	}

	for(auto& thread : list) {
		if(&thread != this && thread.interrupt_level <= interrupt_level) {
			thread.suspend();
		}
	}

	previous_mask = interrupt_mask;
	interrupt_mask = interrupt_level;

	interrupt->unlock();
}

void CThread::interrupt_end()
{
	assert(isCurrent());

	interrupt->lock();

	interrupt_mask = previous_mask;

	for(auto& thread : list) {
		if(&thread != this && thread.interrupt_level <= interrupt_level) {
			thread.resume();
		}
	}

	if(interrupt_mask == 0) {
		resume_main_thread();
	}

	interrupt->unlock();
}

const char* CThread::getCurrentName()
{
	auto cur = pthread_self();
	for(auto& t : list) {
		if(t == cur) {
			return t.name;
		}
	}

	return "";
}
