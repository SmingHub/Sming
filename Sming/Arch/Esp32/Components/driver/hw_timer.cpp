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
#include <hal/timer_ll.h>
#include <esp_intr_alloc.h>
#include <soc/timer_periph.h>

namespace
{
#if ESP_IDF_VERSION_MAJOR >= 5
using timer_group_t = unsigned int;
using timer_idx_t = unsigned int;
#endif

class TimerConfig
{
public:
	TimerConfig(unsigned group, unsigned index)
		: group(timer_group_t(group)), index(timer_idx_t(index)), dev(TIMER_LL_GET_HW(group))
	{
	}

	void begin()
	{
		periph_module_enable(timer_group_periph_signals.groups[group].module);
		enable_counter(false);
		enable_alarm(true);
		set_alarm_value(0);
#if ESP_IDF_VERSION_MAJOR < 5
		timer_ll_set_counter_increase(dev, index, false);
#else
		timer_ll_set_count_direction(dev, index, GPTIMER_COUNT_DOWN);
		timer_ll_set_clock_source(dev, index, GPTIMER_CLK_SRC_DEFAULT);
#endif
	}

	void IRAM_ATTR attach_interrupt(hw_timer_source_type_t source_type, hw_timer_callback_t callback, void* arg)
	{
		if(isr_handle != nullptr) {
			detach_interrupt();
		}

		if(callback == nullptr) {
			return;
		}

		this->callback = callback;

		uint32_t status_reg = reinterpret_cast<uint32_t>(timer_ll_get_intr_status_reg(dev));
		uint32_t mask = 1 << index;
#if ESP_IDF_VERSION_MAJOR < 5
		int source = timer_group_periph_signals.groups[group].t0_irq_id + index;
#else
		int source = timer_group_periph_signals.groups[group].timer_irq_id[index];
#endif
		esp_intr_alloc_intrstatus(source, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_INTRDISABLED, status_reg, mask, timerIsr,
								  this, &isr_handle);
		clear_intr_status();
		enable_intr(true);
	}

	void __forceinline detach_interrupt()
	{
		enable_intr(false);
		esp_intr_free(isr_handle);
		isr_handle = nullptr;
	}

	void __forceinline enable_intr(bool state)
	{
#if ESP_IDF_VERSION_MAJOR < 5
		if(state) {
			timer_ll_intr_enable(dev, index);
		} else {
			timer_ll_intr_disable(dev, index);
		}
#else
		timer_ll_enable_intr(dev, index, state);
#endif
	}

	void __forceinline clear_intr_status()
	{
		timer_ll_clear_intr_status(dev, index);
	}

	void __forceinline set_alarm_value(uint64_t value)
	{
		timer_ll_set_alarm_value(dev, index, value);
	}

	void __forceinline enable_alarm(bool state)
	{
#if ESP_IDF_VERSION_MAJOR < 5
		timer_ll_set_alarm_enable(dev, index, state);
#else
		timer_ll_enable_alarm(dev, index, state);
#endif
	}

	void __forceinline enable_auto_reload(bool state)
	{
#if ESP_IDF_VERSION_MAJOR < 5
		timer_ll_set_auto_reload(dev, index, state);
#else
		timer_ll_enable_auto_reload(dev, index, state);
#endif
		autoload = state;
	}

	void __forceinline set_prescale(uint32_t divider)
	{
#if ESP_IDF_VERSION_MAJOR < 5
		timer_ll_set_divider(dev, index, divider);
#else
		timer_ll_set_clock_prescale(dev, index, divider);
#endif
	}

	void __forceinline set_counter_value(uint64_t value)
	{
#if ESP_IDF_VERSION_MAJOR < 5
		timer_ll_set_counter_value(dev, index, value);
#else
		timer_ll_set_reload_value(dev, index, value);
		timer_ll_trigger_soft_reload(dev, index);
#endif
	}

	uint64_t __forceinline get_counter_value()
	{
#if ESP_IDF_VERSION_MAJOR < 5
		uint64_t val{0};
		timer_ll_get_counter_value(dev, index, &val);
		return val;
#else
		return timer_ll_get_counter_value(dev, index);
#endif
	}

	void __forceinline enable_counter(bool state)
	{
#if ESP_IDF_VERSION_MAJOR < 5
		timer_ll_set_counter_enable(dev, index, state);
#else
		timer_ll_enable_counter(dev, index, state);
#endif
	}

private:
	static void IRAM_ATTR timerIsr(void* arg)
	{
		auto& timer = *static_cast<TimerConfig*>(arg);

		if(timer.callback != nullptr) {
			timer.callback(arg);
		}

		timer.clear_intr_status();

		if(!timer.autoload) {
			timer.enable_counter(false);
		}

		timer.enable_alarm(true);
	}

	timer_group_t group;
	timer_idx_t index;
	timg_dev_t* dev;
	intr_handle_t isr_handle{};
	hw_timer_callback_t callback{nullptr};
	void* arg{nullptr};
	bool autoload{false};
};

TimerConfig timer(HW_TIMER1_GROUP, HW_TIMER1_INDEX);

} // namespace

void IRAM_ATTR hw_timer1_attach_interrupt(hw_timer_source_type_t source_type, hw_timer_callback_t callback, void* arg)
{
	timer.attach_interrupt(source_type, callback, arg);
}

void IRAM_ATTR hw_timer1_detach_interrupt(void)
{
	timer.detach_interrupt();
}

void IRAM_ATTR hw_timer1_enable(hw_timer_clkdiv_t div, hw_timer_intr_type_t intr_type, bool auto_load)
{
	timer.enable_auto_reload(auto_load);
	timer.set_prescale(1 << div);
}

void IRAM_ATTR hw_timer1_write(uint32_t ticks)
{
	timer.set_counter_value(ticks);
	timer.enable_counter(true);
}

void IRAM_ATTR hw_timer1_disable(void)
{
	timer.enable_counter(false);
}

uint32_t hw_timer1_read(void)
{
	return timer.get_counter_value();
}

void hw_timer_init(void)
{
	timer.begin();
}
