#include "include/esp_clk.h"
#include "include/esp_system.h"
#include <hardware/clocks.h>
#include <hardware/pll.h>
#include <esp_attr.h>
#include <hardware/exception.h>
#include <hardware/structs/systick.h>

#define DEFAULT_CPU_FREQ (125 * MHZ)

static volatile uint32_t systick_overflow;

static void IRAM_ATTR systick_overflow_isr()
{
	++systick_overflow;
}

uint32_t IRAM_ATTR esp_get_ccount()
{
	uint32_t ovf = systick_overflow;
	if(ovf != systick_overflow) {
		ovf = systick_overflow;
	}
	// CVR is a down-counter
	return ((1 + ovf) << 24) - systick_hw->cvr;
}

// Fix the peripheral clocks but allow system (CPU) to be varied independently using PLL
void system_init_clocks()
{
	uint vco_freq, post_div1, post_div2;
	check_sys_clock_khz(DEFAULT_CPU_FREQ / 1000U, &vco_freq, &post_div1, &post_div2);

	// Switch to stable system clock before enabling PLL
	clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
					CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * MHZ, 48 * MHZ);

	pll_init(pll_sys, 1, vco_freq, post_div1, post_div2);
	uint32_t freq = vco_freq / (post_div1 * post_div2);

	// CLK_REF = XOSC (12MHz) / 1 = 12MHz
	clock_configure(clk_ref, CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
					0, // No aux mux
					12 * MHZ, 12 * MHZ);

	// CLK SYS = PLL SYS (125MHz) / 1 = 125MHz
	clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
					CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, freq, freq);

	clock_configure(clk_peri,
					0, // Only AUX mux on ADC
					CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * MHZ, 48 * MHZ);

	// Initialise systick for use by esp_get_ccount()
	exception_set_exclusive_handler(SYSTICK_EXCEPTION, systick_overflow_isr);
#ifdef SOC_RP2350
	systick_hw->csr = (1 << M33_SYST_CSR_CLKSOURCE_LSB) // Processor CLK source
					  | M33_SYST_CSR_TICKINT_BITS		// Enable overflow ISR
					  | M33_SYST_CSR_ENABLE_BITS;		// ENABLE
	systick_hw->rvr = M33_SYST_RVR_BITS;				// Reload value when counter hits 0
#else
	systick_hw->csr = (1 << M0PLUS_SYST_CSR_CLKSOURCE_LSB) // Processor CLK source
					  | M0PLUS_SYST_CSR_TICKINT_BITS	   // Enable overflow ISR
					  | M0PLUS_SYST_CSR_ENABLE_BITS;	   // ENABLE
	systick_hw->rvr = M0PLUS_SYST_RVR_BITS;				   // Reload value when counter hits 0
#endif
}

bool system_update_cpu_freq(uint8_t mhz)
{
	uint vco_freq, post_div1, post_div2;
	if(!check_sys_clock_khz(mhz * 1000U, &vco_freq, &post_div1, &post_div2)) {
		return false;
	}

	// Switch to stable system clock before messing with PLL
	clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
					CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * MHZ, 48 * MHZ);

	pll_init(pll_sys, 1, vco_freq, post_div1, post_div2);
	uint32_t freq = vco_freq / (post_div1 * post_div2);

	// CLK SYS = PLL SYS (125MHz) / 1 = 125MHz
	clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
					CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, freq, freq);
	return true;
}

uint8_t system_get_cpu_freq(void)
{
	return clock_get_hz(clk_sys) / MHZ;
}
