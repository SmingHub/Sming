#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t os_signal_t;
typedef uint32_t os_param_t;

typedef struct {
	os_signal_t sig;
	os_param_t par;
} os_event_t;

enum {
	USER_TASK_PRIO_0,
	USER_TASK_PRIO_1,
	USER_TASK_PRIO_2,
	USER_TASK_PRIO_MAX,
};

typedef void (*os_task_t)(os_event_t* e);

bool system_os_task(os_task_t task, uint8_t prio, os_event_t* queue, uint8_t qlen);
bool system_os_post(uint8_t prio, os_signal_t sig, os_param_t par);

// Setup default task queues
void host_init_tasks();

// Hook function to process task queues
void host_service_tasks();

typedef void (*host_task_callback_t)(uint32_t param);

void host_queue_callback(host_task_callback_t callback, uint32_t param);

#ifdef __cplusplus
}
#endif
