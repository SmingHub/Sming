#include "include/esp_tasks.h"
#include <esp_task.h>
#include <debug_progmem.h>

#ifndef DISABLE_NETWORK
#include <lwip/tcpip.h>
#endif

namespace
{
os_task_t taskCallback;
QueueHandle_t eventQueue;

#ifdef DISABLE_NETWORK

void sming_task_loop(void*)
{
	os_event_t evt;
	while(xQueueReceive(eventQueue, &evt, portMAX_DELAY) == pdTRUE) {
		taskCallback(&evt);
	}
}

#else

tcpip_callback_msg* callbackMessage;
volatile bool eventQueueFlag;

void tcpip_message_handler(void*)
{
	eventQueueFlag = false;
	os_event_t evt;
	while(xQueueReceive(eventQueue, &evt, 0) == pdTRUE) {
		taskCallback(&evt);
	}
}

#endif

} // namespace

bool system_os_task(os_task_t callback, uint8_t prio, os_event_t* queue, uint8_t qlen)
{
	if(callback == nullptr || queue == nullptr || qlen == 0) {
		debug_e("TQ: Bad parameters");
		return false;
	}

	if(prio != USER_TASK_PRIO_1) {
		debug_e("TQ: Invalid priority %u", prio);
		return false;
	}

	if(taskCallback != nullptr) {
		debug_w("TQ: Queue %u already initialised", prio);
		return false;
	}

	eventQueue = xQueueCreate(qlen, sizeof(os_event_t));
	if(eventQueue == nullptr) {
		return false;
	}

	taskCallback = callback;

	return true;
}

void start_sming_task_loop()
{
#ifdef DISABLE_NETWORK

#if defined(SOC_ESP32) && !CONFIG_FREERTOS_UNICORE
	constexpr unsigned core_id{1};
#else
	constexpr unsigned core_id{0};
#endif
	xTaskCreatePinnedToCore(sming_task_loop, "Sming", CONFIG_LWIP_TCPIP_TASK_STACK_SIZE, nullptr,
							CONFIG_LWIP_TCPIP_TASK_PRIO, nullptr, core_id);

#else

	callbackMessage = tcpip_callbackmsg_new(tcpip_callback_fn(tcpip_message_handler), nullptr);

#endif
}

bool IRAM_ATTR system_os_post(uint8_t prio, os_signal_t sig, os_param_t par)
{
	if(prio != USER_TASK_PRIO_1) {
		return false;
	}

	os_event_t ev{sig, par};
	BaseType_t woken;
	auto res = xQueueSendToBackFromISR(eventQueue, &ev, &woken);
	if(res != pdTRUE) {
		return false;
	}

#ifndef DISABLE_NETWORK
	if(!callbackMessage) {
		// Message loop not yet active
		return true;
	}
	// If queue isn't empty and we haven't already asked for a tcpip callback, do that now
	if(xQueueIsQueueEmptyFromISR(eventQueue) == pdFALSE && !eventQueueFlag) {
		eventQueueFlag = true;
		auto err = tcpip_callbackmsg_trycallback_fromisr(callbackMessage);
		woken = (err == ERR_NEED_SCHED);
	} else {
		woken = false;
	}
#endif

	portYIELD_FROM_ISR_ARG(woken);

	return true;
}
