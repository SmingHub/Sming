/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Task.h
 *
 * @author mikee47 mike@sillyhouse.net
 *
 *	Sming provides a task queue for applications to use. It can be used to defer processing
 *	from an interrupt service routine, but can also be used to provide a form of multi-tasking.
 *
 *	This class provides a simple way to create a task which runs continuously, or can be started
 *	and stopped as required.
 *
 *	You can see how this class is used in the `Basic_Tasks` sample.
 *
 *
 ****/

#pragma once

#include <SimpleTimer.h>

/**
 * @brief Class to support running a background task
 * @note A task is implemented by overriding the `loop()` method to perform some work.
 * All tasks must co-operate to ensure the system runs smoothly.
 * Note that there is no `yield()` function to call.
 * All tasks share the same stack space.
 */
class Task
{
public:
	/**
	 * @brief State of a task
	 */
	enum class State {
		Suspended,
		Sleeping,
		Running,
	};

	/**
	 * @brief Notification of state change
	 */
	enum class Notify {
		None,
		Suspending,
		Resuming,
		Sleeping,
		Waking,
	};

	virtual ~Task() = default;

	/**
	 * @brief Call to set task running
	 * @retval bool true on success, false if task queue is full
	 */
	bool resume()
	{
		switch(state) {
		case State::Sleeping:
			sleepTimer.stop();
			notification = Notify::Waking;
			state = State::Running;
			return schedule();
		case State::Suspended:
			notification = Notify::Resuming;
			state = State::Running;
			return schedule();
		case State::Running:
			return true;
		default:
			assert(false);
			return false;
		}
	}

	/**
	 * @brief Suspend a task
	 */
	void suspend()
	{
		switch(state) {
		case State::Sleeping:
			sleepTimer.stop();
			notify(Notify::Suspending);
			state = State::Suspended;
			break;
		case State::Running:
			notify(Notify::Suspending);
			state = State::Suspended;
			break;
		case State::Suspended:
			break;
		default:
			assert(false);
		}
	}

	/**
	 * @brief Puts the task to sleep for a while
	 * @param interval Time in milliseconds
	 */
	void sleep(unsigned interval)
	{
		switch(state) {
		case State::Suspended:
		case State::Running:
			sleepTimer.initializeMs(
				interval,
				[](void* param) {
					auto task = static_cast<Task*>(param);
					task->notify(Notify::Waking);
					task->state = State::Running;
					task->service();
				},
				this);
			sleepTimer.startOnce();
			notify(Notify::Sleeping);
			state = State::Sleeping;
			break;
		case State::Sleeping:
			break;
		}
	}

protected:
	/**
	 * @brief Inherited classes override this to perform actual work
	 */
	virtual void loop() = 0;

	/**
	 * @brief Called immediately before calling to loop() to indicate a state change
	 */
	virtual void onNotify([[maybe_unused]] Notify code)
	{
	}

private:
	/*
	 *
	 */
	void notify(Notify code)
	{
		notification = Notify::None;
		onNotify(code);
	}

	/**
	 * @brief Place task onto task queue
	 * @retval bool true on success, false if queue is full
	 */
	bool schedule()
	{
		if(scheduled) {
			return true;
		}

		scheduled = System.queueCallback(
			[](void* param) {
				auto task = reinterpret_cast<Task*>(param);
				task->scheduled = false;
				task->service();
			},
			this);

		return scheduled;
	}

	/*
	 * Executed via task queue
	 */
	void service()
	{
		if(state == State::Running) {
			if(notification != Notify::None) {
				notify(notification);
			}
			loop();
			schedule();
		}
	}

private:
	State state{State::Suspended};	 ///< Current state
	Notify notification{Notify::None}; ///< Code to notify immediately before next loop()
	bool scheduled{false};			   ///< Indicates whether task is currently queued
	SimpleTimer sleepTimer;
};
