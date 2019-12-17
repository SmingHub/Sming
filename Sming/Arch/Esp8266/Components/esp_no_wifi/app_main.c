#include <esp_attr.h>
#include <stdint.h>
#include <espinc/i2c_bbpll.h>
#include <esp_systemapi.h>

extern uint8_t _bss_start, _bss_end;
extern struct rst_info rst_if;

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


void set_pll(void)
{
	// Initial power-on resets for 26MHz crystal, 40MHz clock
	if(rom_i2c_readReg(i2c_bbpll, 4, 1) == 8) { // 8: 40MHz, 136: 26MHz
		// set 80MHz PLL CPU (Q = 26MHz)
		rom_i2c_writeReg(i2c_bbpll, 4, 1, 0x88);
		rom_i2c_writeReg(i2c_bbpll, 4, 2, 0x91);
	}
}

static void user_start(void)
{
	ets_isr_mask(0x3FE); // disable interrupts 1..9
	//	ets_set_user_start (jump_boot); // set the address for a possible reboot on add. ROM-BIOS branches
	// IO_RTC_4 = 0xfe000000;
	sleep_reset_analog_rtcreg_8266(); // spoils the PLL!
	set_pll();

	memset(&_bss_start, 0, &_bss_end - &_bss_start);

	ets_timer_init();

	system_rtc_mem_read(0, &rst_if, sizeof(rst_if));
	uint32 reset_reason = READ_PERI_REG(RTC_SCRATCH0);
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

	// Clear the stack and transfer control to the ROM-BIOS
	__asm__ volatile("movi	a2, 1;"
					 "slli   a1, a2, 30;");
	ets_run();
}
