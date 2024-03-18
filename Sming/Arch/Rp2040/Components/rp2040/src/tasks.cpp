#include "include/esp_tasks_ll.h"
#include <stringutil.h>
#include <debug_progmem.h>
#include <pico/util/queue.h>
#include <sming_attr.h>

namespace
{
class TaskQueue
{
public:
	static void init()
	{
		spinlock = spin_lock_claim_unused(true);
	}

	TaskQueue(os_task_t callback, uint8_t length) : callback(callback)
	{
		queue_init_with_spinlock(&queue, sizeof(os_event_t), length, spinlock);
	}

	~TaskQueue()
	{
		queue_free(&queue);
	}

	bool __forceinline post(os_signal_t sig, os_param_t par)
	{
		os_event_t event{sig, par};
		return queue_try_add(&queue, &event);
	}

	void process()
	{
		// Don't service any newly queued events
		unsigned count = queue_get_level(&queue);
		while(count--) {
			os_event_t event;
			if(queue_try_remove(&queue, &event)) {
				callback(&event);
			}
		}
	}

	explicit operator bool() const
	{
		return callback && queue.data;
	}

private:
	static uint8_t spinlock;
	os_task_t callback;
	queue_t queue;
};

uint8_t TaskQueue::spinlock;

const uint8_t SYSTEM_TASK_PRIO = USER_TASK_PRIO_MAX;
const uint8_t SYSTEM_TASK_QUEUE_LENGTH = 8;

TaskQueue* task_queues[SYSTEM_TASK_PRIO + 1];

}; // namespace

bool system_os_task(os_task_t callback, os_task_priority_t prio, os_event_t* events, uint8_t qlen)
{
	if(prio >= USER_TASK_PRIO_MAX) {
		debug_e("[TQ] Invalid priority %u", prio);
		return false;
	}
	auto& queue = task_queues[prio];
	if(queue != nullptr) {
		debug_e("[TQ] Queue %u already initialised", prio);
		return false;
	}

	queue = new TaskQueue(callback, qlen);
	return queue && *queue;
}

bool IRAM_ATTR system_os_post(os_task_priority_t prio, os_signal_t sig, os_param_t par)
{
	if(prio >= USER_TASK_PRIO_MAX) {
		return false;
	}
	auto& queue = task_queues[prio];
	if(queue == nullptr) {
		return false;
	}

	return task_queues[prio]->post(sig, par);
}

void system_init_tasks()
{
	TaskQueue::init();

	auto systemTaskCallback = [](os_event_t* event) {
		auto callback = system_task_callback_t(event->sig);
		if(callback != nullptr) {
			callback(event->par);
		}
	};

	task_queues[SYSTEM_TASK_PRIO] = new TaskQueue(systemTaskCallback, SYSTEM_TASK_QUEUE_LENGTH);
}

void system_service_tasks()
{
	for(int prio = SYSTEM_TASK_PRIO; prio >= 0; --prio) {
		auto queue = task_queues[prio];
		if(queue != nullptr) {
			queue->process();
		}
	}
}

bool system_queue_callback(system_task_callback_t callback, uint32_t param)
{
	return task_queues[SYSTEM_TASK_PRIO]->post(os_signal_t(callback), param);
}
