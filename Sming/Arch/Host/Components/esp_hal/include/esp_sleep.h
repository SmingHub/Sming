#pragma once

#include <stdint.h>
#include <driver/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum sleep_type {
	NONE_SLEEP_T = 0,
	LIGHT_SLEEP_T,
	MODEM_SLEEP_T,
};

typedef void (*fpm_wakeup_cb)(void);

void system_deep_sleep(uint32_t time_in_us);
bool system_deep_sleep_set_option(uint8_t option);

/* Forced sleep */

void wifi_fpm_open(void);
void wifi_fpm_close(void);
void wifi_fpm_do_wakeup(void);
void wifi_fpm_set_wakeup_cb(fpm_wakeup_cb cb);
sint8 wifi_fpm_do_sleep(uint32_t sleep_time_in_us);
void wifi_fpm_set_sleep_type(enum sleep_type type);
enum sleep_type wifi_fpm_get_sleep_type(void);
void wifi_fpm_auto_sleep_set_in_null_mode(uint8_t req);

/* GPIO */

void wifi_enable_gpio_wakeup(uint32 i, GPIO_INT_TYPE intr_status);
void wifi_disable_gpio_wakeup(void);

/* These aren't defined in the RTOS SDK */

enum sleep_level {
	MIN_SLEEP_T,
	MAX_SLEEP_T,
};

bool wifi_set_sleep_type(enum sleep_type type);
enum sleep_type wifi_get_sleep_type(void);
bool wifi_set_sleep_level(enum sleep_level level);
enum sleep_level wifi_get_sleep_level(void);
bool wifi_set_listen_interval(uint8_t interval);
uint8_t wifi_get_listen_interval(void);

#ifdef __cplusplus
}
#endif
