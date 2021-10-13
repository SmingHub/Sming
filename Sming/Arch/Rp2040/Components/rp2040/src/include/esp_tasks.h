#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t os_signal_t;
typedef uint32_t os_param_t;

typedef struct {
	os_signal_t sig;
	os_param_t par;
} os_event_t;

typedef enum {
	USER_TASK_PRIO_0,
	USER_TASK_PRIO_1,
	USER_TASK_PRIO_2,
	USER_TASK_PRIO_MAX,
} os_task_priority_t;

typedef void (*os_task_t)(os_event_t* e);

bool system_os_task(os_task_t callback, os_task_priority_t prio, os_event_t* events, uint8_t qlen);
bool system_os_post(os_task_priority_t prio, os_signal_t sig, os_param_t par);

#ifdef __cplusplus
}
#endif

#ifdef DRIVER_CODE_INIT
// Setup default task queues
void system_init_tasks();

// Hook function to process task queues
void system_service_tasks();

typedef void (*system_task_callback_t)(os_param_t param);

bool system_queue_callback(system_task_callback_t callback, os_param_t param);
#endif
