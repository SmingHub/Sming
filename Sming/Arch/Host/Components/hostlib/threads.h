/**
 * threads.h - C++ support for threads and related synchronisation primitives using pthread library
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

#include "include/hostlib/hostlib.h"
#include <hostlib/hostmsg.h>
#include <pthread.h>
#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif
#include <cassert>

#if defined(DEBUG_VERBOSE_LEVEL) && (DEBUG_VERBOSE_LEVEL == 3)
#define HOST_THREAD_DEBUG(fmt, ...) host_printf(fmt "\n", ##__VA_ARGS__)
#else
#define HOST_THREAD_DEBUG(fmt, ...)
#endif

/**
 * @brief Wrapper for posix thread mutex
 *
 * Note: Don't use this in application code, use `CMutex` as it guards against interrupt deadlocks.
 */
class CBasicMutex
{
public:
	CBasicMutex()
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&m_priv, &attr);
	}

	~CBasicMutex()
	{
		pthread_mutex_destroy(&m_priv);
	}

	void lock()
	{
		pthread_mutex_lock(&m_priv);
	}

	bool tryLock()
	{
		return pthread_mutex_trylock(&m_priv) == 0;
	}

	void unlock()
	{
		pthread_mutex_unlock(&m_priv);
	}

	int wait(pthread_cond_t& cond)
	{
		return pthread_cond_wait(&cond, &m_priv);
	}

private:
	/*
     * This behaviour allows threads to lock the mutex multiple times whilst
     * blocking other threads. Avoids risk of deadlocks and simplifies code.
     */
	pthread_mutex_t m_priv;
};

/**
 * @brief Wrapper for posix thread mutex with interrupt deadlock protection
 *
 * To simulate interrupts the main thread is suspended, but if this happens whilst it owns
 * a mutex then interrupt code can deadlock the application.
 *
 * This object uses an additional mutex shared with CThread to guard against this.
 */
class CMutex : public CBasicMutex
{
public:
	void lock();
	void unlock();
};

/**
 * @brief Wrapper for posix semaphore
 */
class CSemaphore
{
public:
	CSemaphore()
	{
#ifdef __APPLE__
		m_sem = dispatch_semaphore_create(0);
#else
		sem_init(&m_sem, 0, 0);
#endif
	}

	~CSemaphore()
	{
#ifndef __APPLE__
		sem_destroy(&m_sem);
#endif
	}

	bool post()
	{
#ifdef __APPLE__
		return dispatch_semaphore_signal(m_sem) == 0;
#else
		return sem_post(&m_sem) == 0;
#endif
	}

	bool wait()
	{
#ifdef __APPLE__
		return dispatch_semaphore_wait(m_sem, DISPATCH_TIME_FOREVER) == 0;
#else
		return sem_wait(&m_sem) == 0;
#endif
	}

	bool trywait()
	{
#ifdef __APPLE__
		return dispatch_semaphore_wait(m_sem, DISPATCH_TIME_NOW) == 0;
#else
		return sem_trywait(&m_sem) == 0;
#endif
	}

	bool timedwait(unsigned us);

private:
#ifdef __APPLE__
	dispatch_semaphore_t m_sem;
#else
	sem_t m_sem{};
#endif
};

/**
 * @brief Class to manage separate execution thread for simulating hardware and interrupts.
 */
class CThread
{
public:
	static void startup(unsigned cpulimit = 0);

	/**
	 * @brief Construct a new CThread object
	 * @param name Name of thread (for debugging)
	 * @param interrupt_level Must be > 0. Higher values can interrupt threads of lower levels.
	 */
	CThread(const char* name, unsigned interrupt_level);

	virtual ~CThread();

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

	/**
	 * @brief Determine if running in the context of this thread
	 */
	bool isCurrent() const
	{
		return pthread_equal(pthread_self(), m_thread) != 0;
	}

	/**
	 * @brief A thread calls this method before issuing an 'interrupt'.
	 *
	 * Will block if any another thread is running interrupt code at the same or higher level.
	 * i.e. high-priority interrupts can pre-empty lower-priority ones.
	 */
	void interrupt_begin();

	/**
	 * @brief Signals end of interrupt code and allows other waiting threads to issue interrupts
	 */
	void interrupt_end();

	/**
	 * @brief Prevent all interrupts
	 */
	static void interrupt_lock();

	/**
	 * @brief Resume interrupts
	 */
	static void interrupt_unlock();

	bool operator==(pthread_t other) const
	{
		return pthread_equal(other, m_thread);
	}

	/**
	 * @brief Get name of the currently executing thread
	 */
	static const char* getCurrentName();

protected:
	/**
	 * @brief Inherited classes must implement this method
	 */
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
	const char* name;				///< Helps to identify purpose for debugging
	unsigned interrupt_level;		///< Interrupt level associated with this thread
	unsigned previous_mask{0};		///< Used to restore previous interrupt mask when interrupt ends
	static unsigned interrupt_mask; ///< Current interrupt level
};

/*
 * Called from main loop
 * @param ms Time in milliseconds until next schedule timer event,
 * negative if no timers have been scheduled.
 */
void host_thread_wait(int ms);

/*
 * Cancels wait, e.g. when new event is posted to queue
 */
void host_thread_kick();
