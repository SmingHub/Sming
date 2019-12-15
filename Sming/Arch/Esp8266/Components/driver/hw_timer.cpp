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

/**
 * @brief timer2_ms_flag
 *
 * FRC2 used as reference for NOW() - a macro which reads FRC2_COUNT register
 *
 * eagle_soc.h defines TIMER_CLK_FREQ using a divisor of 256, but this is only the SDK default setting and
 * is changed to 16 when `system_timer_reinit()` is called.
 *
 * The `timer2_ms_flag` indicates the current prescaler setting, however all related timing constants are
 * pre-calculated to avoid un-necessary runtime calculations.
 *
 * Note: This setting is reflected in the FRC2_CTRL register
 * 		 FRC2_CTRL_ADDRESS = 0x28 (omitted from eagle_soc.h).
 */
extern bool timer2_ms_flag;

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

void hw_timer_init(void)
{
	constexpr uint32_t FRC2_ENABLE_TIMER = BIT7;
	WRITE_PERI_REG(FRC2_CTRL_ADDRESS, FRC2_ENABLE_TIMER | HW_TIMER2_CLKDIV);
#ifdef USE_US_TIMER
	timer2_ms_flag = false;
#else
	timer2_ms_flag = true;
#endif
}
