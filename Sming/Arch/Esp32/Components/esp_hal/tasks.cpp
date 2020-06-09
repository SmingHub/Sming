#include "include/esp_tasks.h"
#include <stringutil.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class TaskQueue
{
public:
	TaskQueue(os_task_t callback, os_event_t* events, uint8_t length)
	{
		this->callback = callback;
		this->events = events;
		this->length = length;
		read = count = 0;
	}

	bool post(os_signal_t sig, os_param_t par)
	{
//		mutex.lock();
		bool full = (count == length);
		if(!full) {
			events[(read + count) % length] = os_event_t{sig, par};
			++count;
		}
//		mutex.unlock();
		return !full;
	}

	void process()
	{
		// Don't service any newly queued events
		for(unsigned n = count; n != 0; --n) {
//			mutex.lock();
			auto evt = events[read];
			read = (read + 1) % length;
			--count;
//			mutex.unlock();
			callback(&evt);
		}
	}

private:
//	CMutex mutex;
	os_task_t callback;
	os_event_t* events;
	uint8_t read;
	uint8_t count;
	uint8_t length;
};

static TaskQueue* task_queues[USER_TASK_PRIO_MAX + 1];

const uint8_t HOST_TASK_PRIO = USER_TASK_PRIO_MAX;

bool system_os_task(os_task_t callback, uint8_t prio, os_event_t* events, uint8_t qlen)
{
	if(prio >= USER_TASK_PRIO_MAX) {
//		hostmsg("Invalid priority %u", prio);
		return false;
	}
	auto& queue = task_queues[prio];
	if(queue != nullptr) {
//		hostmsg("Queue %u already initialised", prio);
		return false;
	}

	queue = new TaskQueue(callback, events, qlen);
	return queue != nullptr;
}

bool system_os_post(uint8_t prio, os_signal_t sig, os_param_t par)
{
	if(prio >= USER_TASK_PRIO_MAX) {
//		debug_e("Invalid priority %u", prio);
		return false;
	}
	auto& queue = task_queues[prio];
	if(queue == nullptr) {
//		debug_e("Task queue %u not initialised", prio);
		return false;
	}

	return task_queues[prio]->post(sig, par);
}

void host_init_tasks()
{
	static os_event_t events[32];

	auto hostTaskCallback = [](os_event_t* event) {
		auto callback = host_task_callback_t(event->sig);
		if(callback != nullptr) {
			callback(event->par);
		}
	};

	task_queues[HOST_TASK_PRIO] = new TaskQueue(hostTaskCallback, events, ARRAY_SIZE(events));
}

// TODO: add task process control...
static bool stop_tasks = false;

static SemaphoreHandle_t processSemaphore = NULL;

static void process_task(void *pvParameters)
{
	while(!stop_tasks) {

		if(processSemaphore == NULL) {
			// This should not happen ...
			break;
		}

		if( xSemaphoreTake( processSemaphore, ( TickType_t ) 10 ) == pdTRUE ) {
			for(int prio = HOST_TASK_PRIO; prio >= 0; --prio) {
				auto queue = task_queues[prio];
				if(queue != nullptr) {
					queue->process();
				}
			}
			xSemaphoreGive( processSemaphore );
		}

		vTaskDelay(10 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

void host_service_tasks() {
	vSemaphoreCreateBinary( processSemaphore );
	// TODO: check the priority
	xTaskCreate(process_task, "service_task", 2048, 0, 2, NULL);
}

bool host_queue_callback(host_task_callback_t callback, uint32_t param)
{
	return task_queues[HOST_TASK_PRIO]->post(os_signal_t(callback), param);
}
