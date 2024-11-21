#include "include/esp_tasks.h"
#include <esp_event.h>
#include <debug_progmem.h>

#ifndef DISABLE_NETWORK
#include <lwip/tcpip.h>
#endif

namespace
{
os_task_t taskCallback;

#ifdef DISABLE_NETWORK

ESP_EVENT_DEFINE_BASE(TaskEvt);

void event_handler(void*, esp_event_base_t, int32_t event_id, void* event_data)
{
	os_event_t ev{os_signal_t(event_id), 0};
	if(event_data != nullptr) {
		ev.par = *static_cast<os_param_t*>(event_data);
	}

	taskCallback(&ev);
}

#else

QueueHandle_t eventQueue;
tcpip_callback_msg* callbackMessage;

void tcpip_message_handler(void*)
{
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

#ifdef DISABLE_NETWORK

	auto err = esp_event_handler_instance_register(TaskEvt, ESP_EVENT_ANY_ID, event_handler, nullptr, nullptr);
	if(err != ESP_OK) {
		return false;
	}

#else

	eventQueue = xQueueCreate(qlen, sizeof(os_event_t));
	if(eventQueue == nullptr) {
		return false;
	}

	callbackMessage = tcpip_callbackmsg_new(tcpip_callback_fn(tcpip_message_handler), nullptr);

	if(callbackMessage == nullptr) {
		return false;
	}

#endif

	taskCallback = callback;

	return true;
}

bool IRAM_ATTR system_os_post(uint8_t prio, os_signal_t sig, os_param_t par)
{
	if(prio != USER_TASK_PRIO_1) {
		return false;
	}

#ifdef DISABLE_NETWORK

	esp_err_t err;
	if(par == 0) {
		err = esp_event_isr_post(TaskEvt, sig, nullptr, 0, nullptr);
	} else {
		err = esp_event_isr_post(TaskEvt, sig, &par, sizeof(par), nullptr);
	}
	return (err == ESP_OK);

#else

	os_event_t ev{sig, par};
	auto res = xQueueSendToBack(eventQueue, &ev, 0);
	if(res != pdTRUE) {
		return false;
	}
	auto err = tcpip_callbackmsg_trycallback_fromisr(callbackMessage);
	return err == ERR_OK;

#endif
}
