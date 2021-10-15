/*
 * Used by drivers and system code
 */
#pragma once

#include "esp_tasks.h"

// Setup default task queues
void system_init_tasks();

// Hook function to process task queues
void system_service_tasks();

typedef void (*system_task_callback_t)(os_param_t param);

bool system_queue_callback(system_task_callback_t callback, os_param_t param);
