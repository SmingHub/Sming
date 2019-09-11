/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * hw_timer.cpp - Hardware timer host emulation
 *
 * @author mikee47 <mike@sillyhouse.net>
 *
 ****/

#include <driver/hw_timer.h>
#include <hostlib/threads.h>
#include <sys/time.h>
#include <errno.h>
#include <hostlib/hostmsg.h>
#include <assert.h>
#include <muldiv.h>
#include <esp_system.h>

/* Timer 1 */

class CTimerThread;

/*
 * Simulate interrupts using a secondary thread
 */
class CTimerThread : public CThread
{
public:
	explicit CTimerThread(const char* name) : CThread(name, 3)
	{
		execute();
	}

	~CTimerThread()
	{
		state = terminating;
		sem.post();
	}

	void attach_interrupt(hw_timer_source_type_t source_type, hw_timer_callback_t callback, void* arg)
	{
		stop();
		source_type = source_type;
		this->callback.func = callback;
		this->callback.arg = arg;
	}

	void detach_interrupt()
	{
		stop();
		callback.func = nullptr;
		callback.arg = nullptr;
	}

	void enable(hw_timer_clkdiv_t div, hw_timer_intr_type_t intr_type, bool auto_load)
	{
		stop();
		switch(div) {
		case TIMER_CLKDIV_1:
			prescale = 1;
			break;
		case TIMER_CLKDIV_16:
			prescale = 16;
			break;
		case TIMER_CLKDIV_256:
			prescale = 256;
		default:
			assert(false);
		}
		this->auto_load = auto_load;
	}

	void write(uint32_t ticks)
	{
		stop();

		// Determine interval in microseconds
		divisor = base_ticks_per_us::den * prescale;
		interval = muldiv64(ticks, divisor, base_ticks_per_us::num);
		start_time = now();

		state = running;
		sem.post();
	}

	void stop()
	{
		if(state == running) {
			state = stopped;
			sem.post();
			while(thread_state != state) {
				//
			}
		}
	}

	uint32_t read()
	{
		auto elapsed = now() - start_time;
		if(elapsed >= interval) {
			return 0;
		} else {
			return (interval - elapsed) * base_ticks_per_us::num / divisor;
		}
	}

protected:
	void* thread_routine() override;

	uint64_t now()
	{
		return os_get_nanoseconds() / 1000ULL;
	}

private:
	typedef std::ratio<HW_TIMER_BASE_CLK, 1000000> base_ticks_per_us;
	uint32_t divisor = 1;
	uint32_t frequency = HW_TIMER_BASE_CLK;
	uint64_t start_time = 0;
	uint64_t interval = 0; // In microseconds
	CSemaphore sem;		   // Signals state change
	CMutex mutex;
	enum State { stopped, running, terminating };
	State thread_state = stopped;
	State state = stopped;

	hw_timer_source_type_t source_type = TIMER_FRC1_SOURCE;
	unsigned irq_level = 1;
	struct {
		hw_timer_callback_t func = nullptr;
		void* arg = nullptr;
	} callback;
	unsigned prescale = 1;
	bool auto_load = false;
};

static CTimerThread timer1("Timer1");

void* CTimerThread::thread_routine()
{
#ifdef __WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif

	while(state != terminating) {
		if(state == stopped) {
			thread_state = stopped;
			sem.wait();
			continue;
		}

		/*
		 * Typical scheduler minimum timeslice
		 * @todo Determine this value by asking OS
		 */
		const unsigned SCHED_MIN = 1500;
		if(interval > SCHED_MIN) {
			timeval tv;
			gettimeofday(&tv, nullptr);
			tv.tv_usec += interval - SCHED_MIN;
			timespec to;
			to.tv_sec = tv.tv_sec + tv.tv_usec / 1000000;
			to.tv_nsec = (tv.tv_usec % 1000000) * 1000;
			if(sem.timedwait(&to)) {
				continue; // state changed
			}
			if(errno != ETIMEDOUT) {
				hostmsg("Warning! Timer thread errno = %u", errno);
				break;
			}
		}
		/*
		 * Can't guarantee we won't get pre-empted here but if we've
		 * just come out of a wait state should be fine.
		 */
		while((now() - start_time) < interval && !sem.trywait()) {
			//
		}

		if(state != running) {
			continue;
		}

		interrupt_begin();
		if(callback.func != nullptr) {
			callback.func(callback.arg);
		}
		interrupt_end();

		if(state != running) {
			continue;
		}

		// Setup next time (assuming auto_load)
		if(auto_load) {
			start_time += interval;
		} else {
			thread_state = stopped;
		}
	}

	thread_state = terminating;

	return nullptr;
}

void hw_timer1_attach_interrupt(hw_timer_source_type_t source_type, hw_timer_callback_t callback, void* arg)
{
	timer1.attach_interrupt(source_type, callback, arg);
}

void hw_timer1_enable(hw_timer_clkdiv_t div, hw_timer_intr_type_t intr_type, bool auto_load)
{
	timer1.enable(div, intr_type, auto_load);
}

void hw_timer1_write(uint32_t ticks)
{
	timer1.write(ticks);
}

void hw_timer1_disable()
{
	timer1.stop();
}

void hw_timer1_detach_interrupt()
{
	timer1.detach_interrupt();
}

uint32_t hw_timer1_read()
{
	return timer1.read();
}

uint32_t hw_timer2_read()
{
	using R = std::ratio<HW_TIMER2_CLK, 1000000000ULL>;
	return os_get_nanoseconds() * R::num / R::den;
}
