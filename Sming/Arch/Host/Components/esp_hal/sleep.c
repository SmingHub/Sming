#include "include/esp_sleep.h"

bool system_deep_sleep(uint32_t time_in_us)
{
	(void)time_in_us;
	return false;
}

bool system_deep_sleep_set_option(uint8_t option)
{
	(void)option;
	return false;
}

void wifi_fpm_open(void)
{
}

void wifi_fpm_close(void)
{
}

void wifi_fpm_do_wakeup(void)
{
}

void wifi_fpm_set_wakeup_cb(fpm_wakeup_cb cb)
{
	(void)cb;
}

sint8 wifi_fpm_do_sleep(uint32_t sleep_time_in_us)
{
	(void)sleep_time_in_us;
	return -2; // not enabled
}

void wifi_fpm_set_sleep_type(enum sleep_type type)
{
	(void)type;
}

enum sleep_type wifi_fpm_get_sleep_type(void)
{
	return NONE_SLEEP_T;
}

void wifi_fpm_auto_sleep_set_in_null_mode(uint8_t req)
{
	(void)req;
}

/* GPIO */

void wifi_enable_gpio_wakeup(uint32_t i, GPIO_INT_TYPE intr_status)
{
	(void)i;
	(void)intr_status;
}

void wifi_disable_gpio_wakeup(void)
{
}

/* These aren't defined in the RTOS SDK */

bool wifi_set_sleep_type(enum sleep_type type)
{
	return type == NONE_SLEEP_T;
}

enum sleep_type wifi_get_sleep_type(void)
{
	return NONE_SLEEP_T;
}

bool wifi_set_sleep_level(enum sleep_level level)
{
	(void)level;
	return false;
}

enum sleep_level wifi_get_sleep_level(void)
{
	return MIN_SLEEP_T;
}

bool wifi_set_listen_interval(uint8_t interval)
{
	(void)interval;
	return false;
}

uint8_t wifi_get_listen_interval(void)
{
	return 0;
}
