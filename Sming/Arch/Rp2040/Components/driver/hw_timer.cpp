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
#include <hardware/irq.h>
#include <hardware/regs/intctrl.h>

hw_timer_private_t hw_timer_private;

namespace
{
void IRAM_ATTR timer1_isr()
{
	hw_clear_bits(&timer_hw->intr, BIT(0));
	auto& p = hw_timer_private;
	if(p.timer1_callback != nullptr) {
		p.timer1_callback(p.timer1_arg);
	}
	if(p.timer1_autoload) {
		timer_hw->alarm[0] += p.timer1_ticks;
	}
}

} // namespace

void IRAM_ATTR hw_timer1_attach_interrupt(hw_timer_source_type_t source_type, hw_timer_callback_t callback, void* arg)
{
	(void)source_type;
	auto irq_num = TIMER_ALARM_IRQ_NUM(HW_TIMER_NUM, 0);
	irq_set_enabled(irq_num, false);
	auto& p = hw_timer_private;
	p.timer1_callback = callback;
	p.timer1_arg = arg;
	irq_set_exclusive_handler(irq_num, timer1_isr);
	hw_set_bits(&timer_hw->inte, BIT(0));
	irq_set_enabled(irq_num, true);
}

void hw_timer1_detach_interrupt()
{
	hw_clear_bits(&timer_hw->inte, BIT(0));
	auto irq_num = TIMER_ALARM_IRQ_NUM(HW_TIMER_NUM, 0);
	irq_set_enabled(irq_num, false);
	irq_remove_handler(irq_num, timer1_isr);
}

void IRAM_ATTR hw_timer1_enable(hw_timer_clkdiv_t div, hw_timer_intr_type_t intr_type, bool auto_load)
{
	(void)intr_type;
	auto& p = hw_timer_private;
	p.timer1_clkdiv = div;
	p.timer1_autoload = auto_load;
}

void hw_timer_init()
{
	timer_hardware_alarm_claim(HW_TIMER_INST, 0);
}
