#include <esp_attr.h>
#include <stdint.h>
//#include <xtensa/xtruntime.h>
#include <espinc/i2c_bbpll.h>
#include <esp_systemapi.h>

extern uint8_t _bss_start, _bss_end;

void Cache_Read_Enable(uint32_t, uint32_t, uint32_t);
void Cache_Read_Enable_New(void);
void ets_timer_init(void);
void ets_run(void);

// libphy:phy_sleep.o
void sleep_reset_analog_rtcreg_8266(void);

void user_pre_init(void);
void user_init(void);

void user_fatal_exception_handler(void)
{
	//
}

typedef void (*init_done_cb_t)(void);

static init_done_cb_t __system_init_done;

void system_init_done_cb(init_done_cb_t cb)
{
	__system_init_done = cb;
}

static void user_start(void)
{
	memset(&_bss_start, 0, &_bss_end - &_bss_start);

//	sleep_reset_analog_rtcreg_8266();

	// Required to get things to start up properly
	if(rom_i2c_readReg(i2c_bbpll, 4, 1) == 8) { // 8: 40MHz, 136: 26MHz
		// set 80MHz PLL CPU (Q = 26MHz)
		rom_i2c_writeReg(i2c_bbpll, 4, 1, 0x88);
		rom_i2c_writeReg(i2c_bbpll, 4, 2, 0x91);
		ets_delay_us(150);
	}

	ets_timer_init();

	// Initialise heap
	free(malloc(8));

	user_init();

//	user_pre_init();

	if(__system_init_done) {
		__system_init_done();
	}
}

void IRAM_ATTR call_user_start_local(void)
{
	Cache_Read_Enable(0, 0, 1);
	Cache_Read_Enable_New();

	user_start();

	ets_run();
}
