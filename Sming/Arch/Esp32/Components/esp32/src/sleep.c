#include "include/esp_sleep.h"
#include <rom/gpio.h>

void system_deep_sleep(uint32_t time_in_us)
{
}

bool system_deep_sleep_set_option(uint8_t option)
{
	return false;
}

/* GPIO */

void wifi_enable_gpio_wakeup(uint32_t i, GPIO_INT_TYPE intr_status)
{
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
	return false;
}

enum sleep_level wifi_get_sleep_level(void)
{
	return MIN_SLEEP_T;
}

bool wifi_set_listen_interval(uint8_t interval)
{
	return false;
}

uint8_t wifi_get_listen_interval(void)
{
	return 0;
}
