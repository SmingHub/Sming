/**
 * threads.h - C++ support for threads and related sychronisation primitives using pthread library
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

#pragma once

#include "hostlib.h"
#include "hostmsg.h"
#include <pthread.h>
#include <semaphore.h>

#if defined(DEBUG_VERBOSE_LEVEL) && (DEBUG_VERBOSE_LEVEL == 3)
#define HOST_THREAD_DEBUG(fmt, ...) host_printf(fmt "\n", ##__VA_ARGS__)
#else
#define HOST_THREAD_DEBUG(fmt, ...)
#endif

class CMutex;

class CThread
{
public:
	static void startup();

	// Sets interrupt level for current thread
	static void set_interrupt_level(unsigned new_level);

	CThread(const char* name, unsigned interrupt_level) : name(name), interrupt_level(interrupt_level)
	{
	}

	virtual ~CThread()
	{
		HOST_THREAD_DEBUG("Thread '%s' destroyed", name);
	}

	bool execute()
	{
		return pthread_create(&m_thread, NULL, thread_start, this) == 0;
	}

	bool detach()
	{
		return pthread_detach(m_thread) == 0;
	}

	bool cancel()
	{
		return pthread_cancel(m_thread) == 0;
	}

	void join()
	{
		pthread_join(m_thread, nullptr);
		HOST_THREAD_DEBUG("Thread '%s' complete", name);
	}

	/*
	 * Called at the start of any code which affects framework variables.
	 * Will block if any another thread is running in interrupt context.
	 *
	 * @todo Only block if another thread is running at the same or higher level
	 * i.e. high-priority interrupts can pre-empty lower-priority ones.
	 */
	void interrupt_begin()
	{
		set_interrupt_level(interrupt_level);
	}

	/*
	 * Allows other waiting threads to resume.
	 */
	void interrupt_end()
	{
		set_interrupt_level(0);
	}

	static void interrupt_lock();
	static void interrupt_unlock();

protected:
	virtual void* thread_routine() = 0;

private:
	static void* thread_start(void* param)
	{
		auto thread = static_cast<CThread*>(param);
		HOST_THREAD_DEBUG("Thread '%s' running", thread->name);
		return thread->thread_routine();
	}

private:
	pthread_t m_thread = {0};
	const char* name;		  // Helps to identify purpose for debugging
	unsigned interrupt_level; // Interrupt level associated with this thread
	static CMutex interrupt;
};

class CMutex
{
public:
	~CMutex()
	{
		pthread_mutex_destroy(&m_priv);
	}

	void lock()
	{
		pthread_mutex_lock(&m_priv);
	}
	void unlock()
	{
		pthread_mutex_unlock(&m_priv);
	}

private:
	/*
     * This behaviour allows threads to lock the mutex multiple times whilst
     * blocking other threads. Avoids risk of deadlocks and simplifies code.
     */
	pthread_mutex_t m_priv = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
};

class CSemaphore
{
public:
	CSemaphore()
	{
		sem_init(&m_sem, 0, 0);
	}

	~CSemaphore()
	{
		sem_destroy(&m_sem);
	}

	bool post()
	{
		return sem_post(&m_sem) == 0;
	}

	bool wait()
	{
		return sem_wait(&m_sem) == 0;
	}

	bool trywait()
	{
		return sem_trywait(&m_sem) == 0;
	}

	bool timedwait(const struct timespec* abs_timeout)
	{
		return sem_timedwait(&m_sem, abs_timeout) == 0;
	}

	bool timedwait(unsigned ms)
	{
		timespec to;
		to.tv_sec = ms / 1000;
		to.tv_nsec = (ms % 1000) * 1000000;
		return timedwait(&to);
	}

private:
	sem_t m_sem;
};
