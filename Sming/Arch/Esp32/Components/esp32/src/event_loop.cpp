/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * event_loop.cpp
 *
 * This code replaces the standard IDF event loop with our own, *without* associated task.
 * This not only reduces the system overhead but avoids the need for additional synchronisation
 * management because WiFi events, etc. are all called in the context of the main Sming task.
 */

#include <esp_event.h>

namespace
{
esp_event_loop_handle_t sming_event_loop;
}

esp_event_loop_handle_t sming_create_event_loop()
{
	esp_event_loop_args_t loop_args = {
		.queue_size = CONFIG_ESP_SYSTEM_EVENT_QUEUE_SIZE,
		.task_name = nullptr,
	};

	ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &sming_event_loop));

	return sming_event_loop;
}

namespace
{
#define WRAP(name) esp_err_t __wrap_##name

extern "C" {

WRAP(esp_event_loop_create_default)()
{
	return ESP_ERR_INVALID_STATE;
}

WRAP(esp_event_handler_register)
(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg)
{
	if(sming_event_loop == nullptr) {
		return ESP_ERR_INVALID_STATE;
	}

	return esp_event_handler_register_with(sming_event_loop, event_base, event_id, event_handler, event_handler_arg);
}

WRAP(esp_event_handler_unregister)
(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler)
{
	if(sming_event_loop == nullptr) {
		return ESP_ERR_INVALID_STATE;
	}

	return esp_event_handler_unregister_with(sming_event_loop, event_base, event_id, event_handler);
}

WRAP(esp_event_handler_instance_register)
(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg,
 esp_event_handler_instance_t* instance)
{
	if(sming_event_loop == nullptr) {
		return ESP_ERR_INVALID_STATE;
	}

	return esp_event_handler_instance_register_with(sming_event_loop, event_base, event_id, event_handler,
													event_handler_arg, instance);
}

WRAP(esp_event_handler_instance_unregister)
(esp_event_base_t event_base, int32_t event_id, esp_event_handler_instance_t context)
{
	if(sming_event_loop == nullptr) {
		return ESP_ERR_INVALID_STATE;
	}

	return esp_event_handler_instance_unregister_with(sming_event_loop, event_base, event_id, context);
}

WRAP(esp_event_post)
(esp_event_base_t event_base, int32_t event_id, void* event_data, size_t event_data_size, TickType_t ticks_to_wait)
{
	if(sming_event_loop == nullptr) {
		return ESP_ERR_INVALID_STATE;
	}

	return esp_event_post_to(sming_event_loop, event_base, event_id, event_data, event_data_size, ticks_to_wait);
}

#if CONFIG_ESP_EVENT_POST_FROM_ISR
IRAM_ATTR WRAP(esp_event_isr_post)(esp_event_base_t event_base, int32_t event_id, void* event_data,
								   size_t event_data_size, BaseType_t* task_unblocked)
{
	if(sming_event_loop == nullptr) {
		return ESP_ERR_INVALID_STATE;
	}

	return esp_event_isr_post_to(sming_event_loop, event_base, event_id, event_data, event_data_size, task_unblocked);
}
#endif

} // extern "C"

} // namespace
