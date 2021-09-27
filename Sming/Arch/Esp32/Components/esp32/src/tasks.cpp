#include "include/esp_tasks.h"
#include <esp_event.h>
#include <debug_progmem.h>

namespace
{
ESP_EVENT_DEFINE_BASE(TaskEvt);

os_task_t taskCallback;

} // namespace

bool system_os_task(os_task_t callback, uint8_t prio, os_event_t* events, uint8_t qlen)
{
	auto handler = [](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
		assert(taskCallback != nullptr);

		os_event_t ev{os_signal_t(event_id), 0};
		if(event_data != nullptr) {
			ev.par = *static_cast<os_param_t*>(event_data);
		}

		taskCallback(&ev);
	};

	if(callback == nullptr) {
		debug_e("TQ: Callback missing");
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

	auto err = esp_event_handler_instance_register(TaskEvt, ESP_EVENT_ANY_ID, handler, nullptr, nullptr);
	if(err != ESP_OK) {
		debug_e("TQ: Failed to register handler");
		return false;
	}

	taskCallback = callback;

	debug_i("TQ: Registered %s", TaskEvt);

	return true;
}

bool IRAM_ATTR system_os_post(uint8_t prio, os_signal_t sig, os_param_t par)
{
	if(prio != USER_TASK_PRIO_1) {
		return false;
	}
	esp_err_t err;
	if(par == 0) {
		err = esp_event_isr_post(TaskEvt, sig, nullptr, 0, nullptr);
	} else {
		err = esp_event_isr_post(TaskEvt, sig, &par, sizeof(par), nullptr);
	}
	return (err == ESP_OK);
}
