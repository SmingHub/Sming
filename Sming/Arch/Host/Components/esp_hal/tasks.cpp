#include "include/esp_tasks.h"
#include <hostlib/hostmsg.h>
#include <stringutil.h>

struct task_queue_t {
	os_task_t callback;
	os_event_t* events;
	uint8_t read;
	uint8_t count;
	uint8_t length;

	void init(os_task_t callback, os_event_t* events, uint8_t length)
	{
		this->callback = callback;
		this->events = events;
		this->length = length;
		read = count = 0;
	}

	bool post(os_signal_t sig, os_param_t par)
	{
		if(count == length) {
			return false;
		}

		events[(read + count) % length] = os_event_t{sig, par};
		++count;
		return true;
	}

	void process()
	{
		while(count != 0) {
			auto evt = events[read];
			read = (read + 1) % length;
			--count;
			callback(&evt);
		}
	}
};

static task_queue_t task_queues[USER_TASK_PRIO_MAX + 1];

const uint8_t HOST_TASK_PRIO = USER_TASK_PRIO_MAX;

bool system_os_task(os_task_t callback, uint8_t prio, os_event_t* events, uint8_t qlen)
{
	if(prio >= USER_TASK_PRIO_MAX) {
		hostmsg("Invalid priority %u", prio);
		return false;
	}

	task_queues[prio].init(callback, events, qlen);
	return true;
}

bool system_os_post(uint8_t prio, os_signal_t sig, os_param_t par)
{
	if(prio >= USER_TASK_PRIO_MAX) {
		hostmsg("Invalid priority %u", prio);
		return false;
	}

	return task_queues[prio].post(sig, par);
}

void host_init_tasks()
{
	static os_event_t events[32];

	task_queues[HOST_TASK_PRIO].init(
		[](os_event_t* event) {
			auto callback = host_task_callback_t(event->sig);
			if(callback) {
				callback(event->par);
			}
		},
		events, ARRAY_SIZE(events));
}

void host_service_tasks()
{
	for(int prio = HOST_TASK_PRIO; prio >= 0; --prio) {
		task_queues[prio].process();
	}
}

void host_queue_callback(host_task_callback_t callback, uint32_t param)
{
	task_queues[HOST_TASK_PRIO].post(os_signal_t(callback), param);
}
