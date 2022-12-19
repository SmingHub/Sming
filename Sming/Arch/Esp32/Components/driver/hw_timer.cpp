/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * hw_timer.cpp
 *
 ****/

#include <driver/hw_timer.h>
#include <driver/periph_ctrl.h>
#include <hal/timer_hal.h>
#include <esp_intr_alloc.h>

namespace
{
struct TimerConfig {
	timer_group_t group;
	timer_idx_t index;
	timer_hal_context_t hal;
	intr_handle_t isr_handle;
	hw_timer_callback_t callback;
	void* arg;
	bool autoload;
};

TimerConfig timer;

void IRAM_ATTR timerIsr(void* arg)
{
	auto& timer = *static_cast<TimerConfig*>(arg);

	if(timer.callback != nullptr) {
		timer.callback(arg);
	}

	timer_hal_clear_intr_status(&timer.hal);

	if(!timer.autoload) {
		timer_hal_set_counter_enable(&timer.hal, false);
	}

	timer_hal_set_alarm_enable(&timer.hal, true);
}

} // namespace

void hw_timer1_attach_interrupt(hw_timer_source_type_t source_type, hw_timer_callback_t callback, void* arg)
{
	if(timer.isr_handle != nullptr) {
		hw_timer1_detach_interrupt();
	}

	if(callback == nullptr) {
		return;
	}

	timer.callback = callback;

	uint32_t status_reg{0};
	uint32_t mask{0};
	timer_hal_get_status_reg_mask_bit(&timer.hal, &status_reg, &mask);
	esp_intr_alloc_intrstatus(timer_group_periph_signals.groups[timer.group].t0_irq_id + timer.index,
							  ESP_INTR_FLAG_IRAM, status_reg, mask, timerIsr, &timer, &timer.isr_handle);
	timer_hal_clear_intr_status(&timer.hal);
	timer_hal_intr_enable(&timer.hal);
}

void hw_timer1_detach_interrupt(void)
{
	timer_hal_intr_disable(&timer.hal);
	esp_intr_free(timer.isr_handle);
	timer.isr_handle = nullptr;
}

void hw_timer1_enable(hw_timer_clkdiv_t div, hw_timer_intr_type_t intr_type, bool auto_load)
{
	timer_hal_set_auto_reload(&timer.hal, auto_load);
	timer_hal_set_divider(&timer.hal, 1 << div);
	timer.autoload = auto_load;
}

void IRAM_ATTR hw_timer1_write(uint32_t ticks)
{
	timer_hal_set_counter_value(&timer.hal, ticks);
	timer_hal_set_counter_enable(&timer.hal, true);
}

void IRAM_ATTR hw_timer1_disable(void)
{
	timer_hal_set_counter_enable(&timer.hal, false);
}

uint32_t hw_timer1_read(void)
{
	uint64_t val{0};
	timer_hal_get_counter_value(&timer.hal, &val);
	return val;
}

void hw_timer_init(void)
{
	timer.group = HW_TIMER1_GROUP;
	timer.index = HW_TIMER1_INDEX;
	periph_module_enable(timer_group_periph_signals.groups[timer.group].module);
	timer_hal_init(&timer.hal, timer.group, timer.index);
	timer_hal_set_counter_enable(&timer.hal, false);
	timer_hal_set_alarm_enable(&timer.hal, true);
	timer_hal_set_alarm_value(&timer.hal, 0);
	timer_hal_set_level_int_enable(&timer.hal, true);
	timer_hal_set_counter_increase(&timer.hal, false);
}
