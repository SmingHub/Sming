#include <esp_systemapi.h>
#include <eagle_soc.h>

bool protect_flag;
bool timer2_ms_flag;
uint32_t WdevTimOffSet;
static bool os_print_enabled = true;
struct rst_info rst_if;

// libphy
uint32_t phy_get_vdd33(void);
uint16_t test_tout(bool debug);
void phy_adc_read_fast(uint16_t* adc_addr, uint16_t adc_num, uint8_t adc_clk_div);

uint8_t ets_post(uint8_t prio, os_signal_t sig, os_param_t par);
void ets_task(os_task_t task, uint8_t prio, os_event_t* queue, uint8_t qlen);
void ets_update_cpu_frequency(uint8_t freq);
void system_restart_core(void);

extern uint8_t _bss_start, _bss_end, _data_start, _data_end, _heap_start, _rodata_start, _rodata_end;

void system_print_meminfo(void)
{
#define ADDR(n) ((uint32_t)&n)
	//	const uint32_t data_start = 0x3FFE8000;
	const uint32_t heap_end = 0x3FFFC000;
	os_printf("data  : 0x%x ~ 0x%x, len: %u\n", ADDR(_data_start), ADDR(_data_end),
			  ADDR(_data_end) - ADDR(_data_start));
	os_printf("rodata: 0x%x ~ 0x%x, len: %u\n", ADDR(_rodata_start), ADDR(_bss_start),
			  ADDR(_bss_start) - ADDR(_rodata_start));
	os_printf("bss   : 0x%x ~ 0x%x, len: %u\n", ADDR(_bss_start), ADDR(_heap_start),
			  ADDR(_heap_start) - ADDR(_bss_start));
	os_printf("heap  : 0x%x ~ 0x%x, len: %u\n", ADDR(_heap_start), heap_end, heap_end - ADDR(_heap_start));
#undef ADDR
}

bool system_os_post(uint8_t prio, os_signal_t sig, os_param_t par)
{
	if(prio >= USER_TASK_PRIO_MAX) {
		os_printf("err: post prio >= %u\n", USER_TASK_PRIO_MAX);
		return false;
	}

	return ets_post(prio + 2, sig, par) == 0; // in ets.h, + 2 SDK 1.5.2
}

bool system_os_task(os_task_t task, uint8_t prio, os_event_t* queue, uint8_t qlen)
{
	if(prio >= USER_TASK_PRIO_MAX) {
		os_printf("err: task prio >= %u\n", USER_TASK_PRIO_MAX);
		return false;
	}

	if(qlen == 0 || queue == NULL) {
		os_printf("err: task queue error\n");
		return false;
	}

	ets_task(task, prio + 2, queue, qlen); // + 2 SDK 1.5.2
	return true;
}

uint32_t system_get_rtc_time()
{
	return READ_PERI_REG(RTC_SLP_CNT_VAL);
}

uint16_t system_adc_read(void)
{
	// uint16 test_tout(bool debug) in libphy,  0 - debug sar summ data out off, 1 - debug sar summ data out on
	return test_tout(false);
}

void system_adc_read_fast(uint16_t* adc_addr, uint16_t adc_num, uint8_t adc_clk_div)
{
	phy_adc_read_fast(adc_addr, adc_num, adc_clk_div);
	if(adc_num) {
		for(unsigned i = 0; i < adc_num; i++) {
			if(adc_addr[i] != 0xffff) {
				adc_addr[i] = adc_addr[i] * 12 / 11;
				if(adc_addr[i] > 0x400) {
					adc_addr[i] = 0x400;
				}
			}
		}
	}
}
/*
000002a0 <system_adc_read_fast>:
     2a0:	e0c112               	addi	a1, a1, -32
     2a3:	11c9                	s32i.n	a12, a1, 4
     2a5:	21d9                	s32i.n	a13, a1, 8
     2a7:	31e9                	s32i.n	a14, a1, 12
     2a9:	5129                	s32i.n	a2, a1, 20
     2ab:	41f9                	s32i.n	a15, a1, 16
     2ad:	0109                	s32i.n	a0, a1, 0
     2af:	03fd                	mov.n	a15, a3
     2b1:	fff901               	l32r	a0, 298 <system_adc_read+0x40>
     2b4:	0000c0               	callx0	a0
     2b7:	4fbc                	beqz.n	a15, 2ef <system_adc_read_fast+0x4f>
     2b9:	fff6e1               	l32r	a14, 294 <system_adc_read+0x3c>
     2bc:	0c0c                	movi.n	a12, 0
     2be:	51d8                	l32i.n	a13, a1, 20
     2c0:	90dcd0               	addx2	a13, a12, a13
     2c3:	001d32               	l16ui	a3, a13, 0
     2c6:	1a13e7               	beq	a3, a14, 2e4 <system_adc_read_fast+0x44>
     2c9:	902330               	addx2	a2, a3, a3
     2cc:	1122e0               	slli	a2, a2, 2
     2cf:	b30c                	movi.n	a3, 11
     2d1:	fff201               	l32r	a0, 29c <system_adc_read+0x44>
     2d4:	0000c0               	callx0	a0
     2d7:	f44020               	extui	a4, a2, 0, 16
     2da:	00a402               	movi	a0, 0x400
     2dd:	003d                	mov.n	a3, a0
     2df:	013047               	bltu	a0, a4, 2e4 <system_adc_read_fast+0x44>
     2e2:	043d                	mov.n	a3, a4
     2e4:	005d32               	s16i	a3, a13, 0
     2e7:	cc1b                	addi.n	a12, a12, 1
     2e9:	f4c0c0               	extui	a12, a12, 0, 16
     2ec:	ce9fc7               	bne	a15, a12, 2be <system_adc_read_fast+0x1e>
     2ef:	11c8                	l32i.n	a12, a1, 4
     2f1:	21d8                	l32i.n	a13, a1, 8
     2f3:	31e8                	l32i.n	a14, a1, 12
     2f5:	41f8                	l32i.n	a15, a1, 16
     2f7:	0108                	l32i.n	a0, a1, 0
     2f9:	20c112               	addi	a1, a1, 32
     2fc:	f00d                	ret.n
     2fe:	00                      	.byte 00
     2ff:	00                      	.byte 00
     300:	ff ff 00 00
*/

uint16_t system_get_vdd33(void)
{
	// uint32 phy_get_vdd33(void) in libphy
	return phy_get_vdd33() & 0x3FF;
}
// Fetch enum `flash_size_map` value
unsigned system_get_flash_size_map(void)
{
	uint32_t hdr;
	spi_flash_read(0, &hdr, sizeof(hdr));
	return hdr >> 28;
}

void system_restart_local(void)
{
	system_restart_core();
}
/*
35c: R_XTENSA_32	clockgate_watchdog
360: R_XTENSA_32	pm_open_rf
364: R_XTENSA_32	system_rtc_mem_read
368: R_XTENSA_32	ets_memset
36c: R_XTENSA_32	system_rtc_mem_write
370: R_XTENSA_32	system_restart_hook
374: R_XTENSA_32	user_uart_wait_tx_fifo_empty
378: R_XTENSA_32	user_uart_wait_tx_fifo_empty
37c: R_XTENSA_32	ets_intr_lock
380: R_XTENSA_32	system_restart_core

00000384 <system_restart_local>:
384:	420c                	movi.n	a2, 4
386:	d0c112               	addi	a1, a1, -48
389:	81c9                	s32i.n	a12, a1, 32
38b:	7109                	s32i.n	a0, a1, 28
38d:	ffcc45               	call0	54 <system_set_os_print+0x50>
38d: R_XTENSA_SLOT0_OP	.irom0.text+0x54
390:	ffebc1               	l32r	a12, 33c <system_restart_hook+0x4>
390: R_XTENSA_SLOT0_OP	.irom0.text+0x33c
393:	160266               	bnei	a2, -1, 3ad <system_restart_local+0x29>
393: R_XTENSA_SLOT0_OP	.irom0.text+0x3ad
396:	020c                	movi.n	a2, 0
398:	fff101               	l32r	a0, 35c <system_restart_hook+0x24>
398: R_XTENSA_SLOT0_OP	.irom0.text+0x35c
398: R_XTENSA_ASM_EXPAND	clockgate_watchdog
39b:	0000c0               	callx0	a0
39e:	ffe801               	l32r	a0, 340 <system_restart_hook+0x8>
39e: R_XTENSA_SLOT0_OP	.irom0.text+0x340
3a1:	0020c0               	memw
3a4:	866c02               	s32i	a0, a12, 0x218
3a7:	ffee01               	l32r	a0, 360 <system_restart_hook+0x28>
3a7: R_XTENSA_SLOT0_OP	.irom0.text+0x360
3a7: R_XTENSA_ASM_EXPAND	pm_open_rf
3aa:	0000c0               	callx0	a0
3ad:	020c                	movi.n	a2, 0
3af:	013d                	mov.n	a3, a1
3b1:	c41c                	movi.n	a4, 28
3b3:	ffec01               	l32r	a0, 364 <system_restart_hook+0x2c>
3b3: R_XTENSA_SLOT0_OP	.irom0.text+0x364
3b3: R_XTENSA_ASM_EXPAND	system_rtc_mem_read
3b6:	0000c0               	callx0	a0
3b9:	0108                	l32i.n	a0, a1, 0
3bb:	273026               	beqi	a0, 3, 3e6 <system_restart_local+0x62>
3bb: R_XTENSA_SLOT0_OP	.irom0.text+0x3e6
3be:	242026               	beqi	a0, 2, 3e6 <system_restart_local+0x62>
3be: R_XTENSA_SLOT0_OP	.irom0.text+0x3e6
3c1:	012d                	mov.n	a2, a1
3c3:	030c                	movi.n	a3, 0
3c5:	c41c                	movi.n	a4, 28
3c7:	ffe801               	l32r	a0, 368 <system_restart_hook+0x30>
3c7: R_XTENSA_SLOT0_OP	.irom0.text+0x368
3c7: R_XTENSA_ASM_EXPAND	ets_memset
3ca:	0000c0               	callx0	a0
3cd:	420c                	movi.n	a2, 4
3cf:	ffdd31               	l32r	a3, 344 <system_restart_hook+0xc>
3cf: R_XTENSA_SLOT0_OP	.irom0.text+0x344
3d2:	0020c0               	memw
3d5:	c41c                	movi.n	a4, 28
3d7:	4c6322               	s32i	a2, a3, 0x130
3da:	0129                	s32i.n	a2, a1, 0
3dc:	013d                	mov.n	a3, a1
3de:	020c                	movi.n	a2, 0
3e0:	ffe301               	l32r	a0, 36c <system_restart_hook+0x34>
3e0: R_XTENSA_SLOT0_OP	.irom0.text+0x36c
3e0: R_XTENSA_ASM_EXPAND	system_rtc_mem_write
3e3:	0000c0               	callx0	a0
3e6:	012d                	mov.n	a2, a1
3e8:	ffe201               	l32r	a0, 370 <system_restart_hook+0x38>
3e8: R_XTENSA_SLOT0_OP	.irom0.text+0x370
3e8: R_XTENSA_ASM_EXPAND	system_restart_hook
3eb:	0000c0               	callx0	a0
3ee:	020c                	movi.n	a2, 0
3f0:	ffd631               	l32r	a3, 348 <system_restart_hook+0x10>
3f0: R_XTENSA_SLOT0_OP	.irom0.text+0x348
3f3:	ffe001               	l32r	a0, 374 <system_restart_hook+0x3c>
3f3: R_XTENSA_SLOT0_OP	.irom0.text+0x374
3f3: R_XTENSA_ASM_EXPAND	user_uart_wait_tx_fifo_empty
3f6:	0000c0               	callx0	a0
3f9:	120c                	movi.n	a2, 1
3fb:	ffd431               	l32r	a3, 34c <system_restart_hook+0x14>
3fb: R_XTENSA_SLOT0_OP	.irom0.text+0x34c
3fe:	ffde01               	l32r	a0, 378 <system_restart_hook+0x40>
3fe: R_XTENSA_SLOT0_OP	.irom0.text+0x378
3fe: R_XTENSA_ASM_EXPAND	user_uart_wait_tx_fifo_empty
401:	0000c0               	callx0	a0
404:	000005               	call0	408 <system_restart_local+0x84>
404: R_XTENSA_SLOT0_OP	system_uart_de_swap
407:	ffdd01               	l32r	a0, 37c <system_restart_hook+0x44>
407: R_XTENSA_SLOT0_OP	.irom0.text+0x37c
407: R_XTENSA_ASM_EXPAND	ets_intr_lock
40a:	0000c0               	callx0	a0
40d:	ffd151               	l32r	a5, 354 <system_restart_hook+0x1c>
40d: R_XTENSA_SLOT0_OP	.irom0.text+0x354
410:	ffd001               	l32r	a0, 350 <system_restart_hook+0x18>
410: R_XTENSA_SLOT0_OP	.irom0.text+0x350
413:	0020c0               	memw
416:	862cb2               	l32i	a11, a12, 0x218
419:	20bb00               	or	a11, a11, a0
41c:	0020c0               	memw
41f:	866cb2               	s32i	a11, a12, 0x218
422:	ffcda1               	l32r	a10, 358 <system_restart_hook+0x20>
422: R_XTENSA_SLOT0_OP	.irom0.text+0x358
425:	0020c0               	memw
428:	862c92               	l32i	a9, a12, 0x218
42b:	1099a0               	and	a9, a9, a10
42e:	0020c0               	memw
431:	866c92               	s32i	a9, a12, 0x218
434:	280c                	movi.n	a8, 2
436:	0020c0               	memw
439:	d22572               	l32i	a7, a5, 0x348
43c:	207780               	or	a7, a7, a8
43f:	0020c0               	memw
442:	d26572               	s32i	a7, a5, 0x348
445:	d67c                	movi.n	a6, -3
447:	0020c0               	memw
44a:	d22542               	l32i	a4, a5, 0x348
44d:	104460               	and	a4, a4, a6
450:	0020c0               	memw
453:	d26542               	s32i	a4, a5, 0x348
456:	81c8                	l32i.n	a12, a1, 32
458:	ffca01               	l32r	a0, 380 <system_restart_hook+0x48>
458: R_XTENSA_SLOT0_OP	.irom0.text+0x380
458: R_XTENSA_ASM_EXPAND	system_restart_core
45b:	0000c0               	callx0	a0
45e:	7108                	l32i.n	a0, a1, 28
460:	30c112               	addi	a1, a1, 48
463:	f00d                	ret.n
*/

struct rst_info* system_get_rst_info(void)
{
	return &rst_if;
}

bool spi_flash_erase_sector_check(void)
{
	return true;
}

bool system_update_cpu_freq(uint8_t freq)
{
	if(freq == SYS_CPU_80MHZ) {
		REG_CLR_BIT(DPORT_CTL_REG, DPORT_CTL_DOUBLE_CLK);
	} else {
		REG_SET_BIT(DPORT_CTL_REG, DPORT_CTL_DOUBLE_CLK);
	}
	ets_update_cpu_frequency(freq);
	return true;
}

void system_set_os_print(uint8_t onoff)
{
	os_print_enabled = onoff;
}

uint8_t system_get_os_print(void)
{
	return os_print_enabled;
}

void ets_write_char(char ch);
int ets_vprintf(void* routine, const char* format, va_list arg);

int os_printf_plus(const char* format, ...)
{
	if(!os_print_enabled) {
		return 0;
	}

	va_list args;
	va_start(args, format);
	int n;

	if(isFlashPtr(format)) {
		char fmtbuf[1024];
		size_t len = strlen_P(format) + 1;
		if(len > sizeof(fmtbuf)) {
			len = sizeof(fmtbuf);
		}
		memcpy_aligned(fmtbuf, format, ALIGNUP(len));
		fmtbuf[len - 1] = '\0';
		n = ets_vprintf(ets_write_char, fmtbuf, args);
	} else {
		n = ets_vprintf(ets_write_char, format, args);
	}

	va_end(args);

	return n;
}
