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
#include <pthread.h>
#include <semaphore.h>

class CThread
{
public:
	virtual ~CThread()
	{
		join();
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
	}

protected:
	virtual void* thread_routine() = 0;

private:
	pthread_t m_thread = {0};
	static void* thread_start(void* param)
	{
		return ((CThread*)param)->thread_routine();
	}
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
