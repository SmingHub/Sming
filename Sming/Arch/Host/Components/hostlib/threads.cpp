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
#include <csignal>
#include <sys/time.h>
#include <cerrno>

unsigned CThread::interrupt_mask;

namespace
{
pthread_t mainThread;
CBasicMutex* interrupt;
pthread_cond_t interruptCond = PTHREAD_COND_INITIALIZER;

#ifdef __WIN32

/*
 * pthread-win32 implements semaphores with a mutex.
 * If it's locked when the main thread is suspended, calling sem_post() from another thread results in deadlock.
 * Use a native Windows semaphore to avoid this issue.
 */
HANDLE host_thread_semaphore;

#else

CSemaphore host_thread_semaphore;
volatile bool mainThreadSignalled;
int pauseSignal;
int resumeSignal;

#ifndef __APPLE__
timer_t signalTimer;
#endif

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
#ifdef __APPLE__
		while(mainThreadSignalled) {
			sched_yield();
		}
#else
		struct timespec ts = {0, long(0.1e9)};
		struct itimerspec its = {ts, ts};
		timer_settime(signalTimer, 0, &its, nullptr);
		while(mainThreadSignalled) {
			pause();
		}
		its = {};
		timer_settime(signalTimer, 0, &its, nullptr);
#endif
	} else if(sig == resumeSignal) {
		mainThreadSignalled = false;
	} else if(sig == SIGALRM) {
	} else {
		assert(false);
	}
}

#endif

[[maybe_unused]] bool isMainThread()
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
	int err = pthread_kill(mainThread, pauseSignal);
	(void)err;
	assert(err == 0);
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
	int err = pthread_kill(mainThread, resumeSignal);
	(void)err;
	assert(err == 0);
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

bool CSemaphore::timedwait(unsigned us)
{
#ifdef __APPLE__
	dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, us * 1000);
	if(dispatch_semaphore_wait(m_sem, time) == 0) {
		return true;
	}
	errno = ETIMEDOUT;
	return false;
#else
	struct timespec ts {
	};
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t ns = ts.tv_nsec + uint64_t(us) * 1000;
	ts.tv_sec += ns / 1000000000;
	ts.tv_nsec = ns % 1000000000;
	return sem_timedwait(&m_sem, &ts) == 0;
#endif
}

void CThread::startup([[maybe_unused]] unsigned cpulimit)
{
#ifndef __APPLE__
	if(cpulimit != 0) {
		cpu_set_t set;
		CPU_ZERO(&set);
		for(unsigned i = 0; i < cpulimit; ++i) {
			CPU_SET(i, &set);
		}
		if(sched_setaffinity(getpid(), sizeof(set), &set) == 0) {
			host_debug_i("Using max. %u CPUs", cpulimit);
		} else {
			host_debug_e("ERROR! Failed to set CPU affinity");
		}
	}
#endif

	mainThread = pthread_self();
	interrupt = new CBasicMutex;

#ifdef __WIN32
	host_thread_semaphore = CreateSemaphore(nullptr, 0, 1024, nullptr);
#elif defined(__APPLE__)
	pauseSignal = SIGUSR1;
	resumeSignal = SIGUSR2;
	signal(pauseSignal, signal_handler);
	signal(resumeSignal, signal_handler);
	signal(SIGALRM, signal_handler);
#else
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
}

CThread::~CThread()
{
	HOST_THREAD_DEBUG("Thread '%s' destroyed", name);
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

void CThread::interrupt_begin()
{
	assert(isCurrent());

	// Block until all equal or higher interrupt levels are done
	interrupt->lock();
	while(interrupt_level <= interrupt_mask) {
		interrupt->wait(interruptCond);
	}
	assert(interrupt_level > interrupt_mask);

	if(interrupt_mask == 0) {
		suspend_main_thread();
	}

	previous_mask = interrupt_mask;
	interrupt_mask = interrupt_level;

	interrupt->unlock();
	pthread_cond_signal(&interruptCond);
}

void CThread::interrupt_end()
{
	assert(isCurrent());

	interrupt->lock();

	interrupt_mask = previous_mask;

	if(interrupt_mask == 0) {
		resume_main_thread();
	}

	interrupt->unlock();
}

void host_thread_wait(int ms)
{
	constexpr int SCHED_WAIT{2};
	if(ms >= 0 && ms <= SCHED_WAIT) {
		return;
	}
#ifdef __WIN32
	WaitForSingleObject(host_thread_semaphore, (ms < 0) ? INFINITE : ms - SCHED_WAIT);
#else
	if(ms < 0) {
		host_thread_semaphore.wait();
	} else if(ms > SCHED_WAIT) {
		host_thread_semaphore.timedwait((ms - SCHED_WAIT) * 1000);
	}
#endif
}

void host_thread_kick()
{
#ifdef __WIN32
	ReleaseSemaphore(host_thread_semaphore, 1, nullptr);
#else
	host_thread_semaphore.post();
#endif
}
