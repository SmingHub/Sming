#pragma once
#include <c_types.h>

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
	USER_TASK_PRIO_1,
};

typedef void (*os_task_t)(os_event_t* e);

bool system_os_task(os_task_t task, uint8_t prio, os_event_t* queue, uint8_t qlen);
bool system_os_post(uint8_t prio, os_signal_t sig, os_param_t par);

#ifdef __cplusplus
}
#endif
