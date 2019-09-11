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

static struct {
	hw_timer_callback_t func = nullptr;
	void* arg = nullptr;
} nmi_callback;

static void IRAM_ATTR nmi_handler()
{
	nmi_callback.func(nmi_callback.arg);
}

void hw_timer1_attach_interrupt(hw_timer_source_type_t source_type, hw_timer_callback_t callback, void* arg)
{
	if(source_type == TIMER_NMI_SOURCE) {
		if(arg == NULL) {
			ETS_FRC_TIMER1_NMI_INTR_ATTACH(reinterpret_cast<void (*)()>(callback));
		} else {
			nmi_callback.func = callback;
			nmi_callback.arg = arg;
			ETS_FRC_TIMER1_NMI_INTR_ATTACH(nmi_handler);
		}
	} else {
		ETS_FRC_TIMER1_INTR_ATTACH(callback, arg);
	}
}
