#include <esp_systemapi.h>
#include <eagle_soc.h>

bool protect_flag;
bool timer2_ms_flag;
uint32_t system_param_sector_start;
uint32_t WdevTimOffSet;
static bool os_print_enabled = true;

void ets_update_cpu_frequency(uint8_t freq);

uint8_t system_upgrade_userbin_check(void)
{
	return 0x00;
}

uint32 system_get_time(void)
{
#define WPS_TIME_REG 0x3ff20c00
	return READ_PERI_REG(WPS_TIME_REG) + WdevTimOffSet;
}

bool system_rtc_mem_write(uint8 des_addr, const void* src_addr, uint16 save_size)
{
	return false;
}

/*
Disassembly of section .text.system_rtc_mem_write:

00000000 <system_rtc_mem_write-0x4>:
   0:	00 0e 00 60

00000004 <system_rtc_mem_write>:
   4:	bfa052               	movi	a5, 191
   7:	143527               	bltu	a5, a2, 1f <system_rtc_mem_write+0x1b>
   a:	139c                	beqz.n	a3, 1f <system_rtc_mem_write+0x1b>
   c:	370c                	movi.n	a7, 3
   e:	146030               	extui	a6, a3, 0, 2
  11:	a6cc                	bnez.n	a6, 1f <system_rtc_mem_write+0x1b>
  13:	1192e0               	slli	a9, a2, 2
  16:	00a382               	movi	a8, 0x300
  19:	c08890               	sub	a8, a8, a9
  1c:	03a847               	bge	a8, a4, 23 <system_rtc_mem_write+0x1f>
  1f:	020c                	movi.n	a2, 0
  21:	f00d                	ret.n
  23:	0a0747               	bnone	a7, a4, 31 <system_rtc_mem_write+0x2d>
  26:	450c                	movi.n	a5, 4
  28:	414240               	srli	a4, a4, 2
  2b:	a04450               	addx4	a4, a4, a5
  2e:	f44040               	extui	a4, a4, 0, 16
  31:	418240               	srli	a8, a4, 2
  34:	e89c                	beqz.n	a8, 56 <system_rtc_mem_write+0x52>
  36:	fff271               	l32r	a7, 0 <system_rtc_mem_write-0x4>
  39:	040c                	movi.n	a4, 0
  3b:	624a                	add.n	a6, a2, a4
  3d:	a05430               	addx4	a5, a4, a3
  40:	a06670               	addx4	a6, a6, a7
  43:	0020c0               	memw
  46:	0558                	l32i.n	a5, a5, 0
  48:	0020c0               	memw
  4b:	c06652               	s32i	a5, a6, 0x300
  4e:	441b                	addi.n	a4, a4, 1
  50:	744040               	extui	a4, a4, 0, 8
  53:	e43487               	bltu	a4, a8, 3b <system_rtc_mem_write+0x37>
  56:	120c                	movi.n	a2, 1
  58:	f00d                	ret.n
*/

bool system_rtc_mem_read(uint8 src_addr, void* des_addr, uint16 load_size)
{
	return false;
}
/*
Disassembly of section .text.system_rtc_mem_read:

00000000 <system_rtc_mem_read-0x4>:
   0:	00 0e 00 60 // rtc_sys_info

00000004 <system_rtc_mem_read>:
   4:	bfa052               	movi	a5, 191
   7:	143527               	bltu	a5, a2, 1f <system_rtc_mem_read+0x1b>
			7: R_XTENSA_SLOT0_OP	.text.system_rtc_mem_read+0x1f
   a:	139c                	beqz.n	a3, 1f <system_rtc_mem_read+0x1b>
			a: R_XTENSA_SLOT0_OP	.text.system_rtc_mem_read+0x1f
   c:	370c                	movi.n	a7, 3
   e:	146030               	extui	a6, a3, 0, 2
  11:	a6cc                	bnez.n	a6, 1f <system_rtc_mem_read+0x1b>
			11: R_XTENSA_SLOT0_OP	.text.system_rtc_mem_read+0x1f
  13:	1192e0               	slli	a9, a2, 2
  16:	00a382               	movi	a8, 0x300
  19:	c08890               	sub	a8, a8, a9
  1c:	03a847               	bge	a8, a4, 23 <system_rtc_mem_read+0x1f>
			1c: R_XTENSA_SLOT0_OP	.text.system_rtc_mem_read+0x23
  1f:	020c                	movi.n	a2, 0
  21:	f00d                	ret.n
  23:	0a0747               	bnone	a7, a4, 31 <system_rtc_mem_read+0x2d>
			23: R_XTENSA_SLOT0_OP	.text.system_rtc_mem_read+0x31
  26:	450c                	movi.n	a5, 4
  28:	414240               	srli	a4, a4, 2
  2b:	a04450               	addx4	a4, a4, a5
  2e:	f44040               	extui	a4, a4, 0, 16
  31:	418240               	srli	a8, a4, 2
  34:	e89c                	beqz.n	a8, 56 <system_rtc_mem_read+0x52>
			34: R_XTENSA_SLOT0_OP	.text.system_rtc_mem_read+0x56
  36:	fff271               	l32r	a7, 0 <system_rtc_mem_read-0x4>
			36: R_XTENSA_SLOT0_OP	.text.system_rtc_mem_read
  39:	040c                	movi.n	a4, 0
  3b:	524a                	add.n	a5, a2, a4
  3d:	a06430               	addx4	a6, a4, a3
  40:	a05570               	addx4	a5, a5, a7
  43:	0020c0               	memw
  46:	c02552               	l32i	a5, a5, 0x300
  49:	0020c0               	memw
  4c:	0659                	s32i.n	a5, a6, 0
  4e:	441b                	addi.n	a4, a4, 1
  50:	744040               	extui	a4, a4, 0, 8
  53:	e43487               	bltu	a4, a8, 3b <system_rtc_mem_read+0x37>
			53: R_XTENSA_SLOT0_OP	.text.system_rtc_mem_read+0x3b
  56:	120c                	movi.n	a2, 1
  58:	f00d                	ret.n
*/

uint16 system_adc_read(void)
{
	return 0;
}
/*
00000258 <system_adc_read>:
     258:	020c                	movi.n	a2, 0
     25a:	f0c112               	addi	a1, a1, -16
     25d:	0109                	s32i.n	a0, a1, 0
     25f:	fffc01               	l32r	a0, 250 <system_pp_recycle_rx_pkt+0x18>
     262:	0000c0               	callx0	a0
     265:	fff901               	l32r	a0, 24c <system_pp_recycle_rx_pkt+0x14>
     268:	f42020               	extui	a2, a2, 0, 16
     26b:	1b1207               	beq	a2, a0, 28a <system_adc_read+0x32>
     26e:	b30c                	movi.n	a3, 11
     270:	902220               	addx2	a2, a2, a2
     273:	1122e0               	slli	a2, a2, 2
     276:	fff701               	l32r	a0, 254 <system_pp_recycle_rx_pkt+0x1c>
     279:	0000c0               	callx0	a0
     27c:	f42020               	extui	a2, a2, 0, 16
     27f:	00a402               	movi	a0, 0x400
     282:	013027               	bltu	a0, a2, 287 <system_adc_read+0x2f>
     285:	020d                	mov.n	a0, a2
     287:	f42000               	extui	a2, a0, 0, 16
     28a:	0108                	l32i.n	a0, a1, 0
     28c:	10c112               	addi	a1, a1, 16
     28f:	f00d                	ret.n
     291:	00                      	.byte 00
     292:	00                      	.byte 00
     293:	00                      	.byte 00
     294:	ff ff 00 00
*/

void system_adc_read_fast(uint16* adc_addr, uint16 adc_num, uint8 adc_clk_div)
{
	//
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

uint16 system_get_vdd33(void)
{
	return 0;
}
/*
0000030c <system_get_vdd33>:
     30c:	f0c112               	addi	a1, a1, -16
     30f:	0109                	s32i.n	a0, a1, 0
     311:	fffc01               	l32r	a0, 304 <system_adc_read_fast+0x64>
     314:	0000c0               	callx0	a0
     317:	fffa01               	l32r	a0, 300 <system_adc_read_fast+0x60>
     31a:	f42020               	extui	a2, a2, 0, 16
     31d:	101207               	beq	a2, a0, 331 <system_get_vdd33+0x25>
     320:	b30c                	movi.n	a3, 11
     322:	902220               	addx2	a2, a2, a2
     325:	1122e0               	slli	a2, a2, 2
     328:	fff801               	l32r	a0, 308 <system_adc_read_fast+0x68>
     32b:	0000c0               	callx0	a0
     32e:	f42020               	extui	a2, a2, 0, 16
     331:	0108                	l32i.n	a0, a1, 0
     333:	10c112               	addi	a1, a1, 16
     336:	f00d                	ret.n
*/

/*
00000504 <system_restore>:
     504:	9ca422               	movi	a2, 0x49c
     507:	d0a142               	movi	a4, 0x1d0
     50a:	fff631               	l32r	a3, 4e4 <system_restart+0x58>
     50d:	050c                	movi.n	a5, 0
     50f:	f0c112               	addi	a1, a1, -16
     512:	11c9                	s32i.n	a12, a1, 4
     514:	0109                	s32i.n	a0, a1, 0
     516:	fff701               	l32r	a0, 4f4 <system_restart+0x68>
     519:	0000c0               	callx0	a0
     51c:	02cd                	mov.n	a12, a2
     51e:	32bc                	beqz.n	a2, 555 <system_restore+0x51>
     520:	ffa032               	movi	a3, 255
     523:	9ca442               	movi	a4, 0x49c
     526:	fff401               	l32r	a0, 4f8 <system_restart+0x6c>
     529:	0000c0               	callx0	a0
     52c:	0c2d                	mov.n	a2, a12
     52e:	ffee31               	l32r	a3, 4e8 <system_restart+0x5c>
     531:	840c                	movi.n	a4, 8
     533:	fff201               	l32r	a0, 4fc <system_restart+0x70>
     536:	0000c0               	callx0	a0
     539:	0c3d                	mov.n	a3, a12
     53b:	ffec21               	l32r	a2, 4ec <system_restart+0x60>
     53e:	9ca442               	movi	a4, 0x49c
     541:	021222               	l16ui	a2, a2, 4
     544:	000005               	call0	548 <system_restore+0x44>
     547:	0c2d                	mov.n	a2, a12
     549:	ffe931               	l32r	a3, 4f0 <system_restart+0x64>
     54c:	dba142               	movi	a4, 0x1db
     54f:	ffec01               	l32r	a0, 500 <system_restart+0x74>
     552:	0000c0               	callx0	a0
     555:	11c8                	l32i.n	a12, a1, 4
     557:	0108                	l32i.n	a0, a1, 0
     559:	10c112               	addi	a1, a1, 16
     55c:	f00d                	ret.n
*/

// Fetch enum `flash_size_map` value
unsigned system_get_flash_size_map(void)
{
	uint32_t hdr;
	spi_flash_read(0, &hdr, sizeof(hdr));
	return hdr >> 28;
}
/*
     55e:	00                      	.byte 00
     55f:	00                      	.byte 00
     560:	00 00 00 00
			560: R_XTENSA_32	spi_flash_read

00000564 <system_get_flash_size_map>:
     564:	020c                	movi.n	a2, 0
     566:	840c                	movi.n	a4, 8
     568:	f0c112               	addi	a1, a1, -16
     56b:	013d                	mov.n	a3, a1
     56d:	2109                	s32i.n	a0, a1, 8
     56f:	fffc01               	l32r	a0, 560 <system_restore+0x5c>
			56f: R_XTENSA_SLOT0_OP	.irom0.text+0x560
			56f: R_XTENSA_ASM_EXPAND	spi_flash_read
     572:	0000c0               	callx0	a0
     575:	0128                	l32i.n	a2, a1, 0
     577:	2108                	l32i.n	a0, a1, 8
     579:	352c20               	extui	a2, a2, 28, 4
     57c:	10c112               	addi	a1, a1, 16
     57f:	f00d                	ret.n
*/

void system_restart_core(void)
{
	//
}
/*
     Disassembly of section .text.system_restart_core:

     00000000 <system_restart_core-0x14>:
        0:	00 00 00 00
     			0: R_XTENSA_32	flashchip
        4:	00 fe ef 3f
        8:	80 00 00 40
     	...
     			c: R_XTENSA_32	Wait_SPI_Idle
     			10: R_XTENSA_32	Cache_Read_Disable

     00000014 <system_restart_core>:
       14:	fffb21               	l32r	a2, 0 <system_restart_core-0x14>
     			14: R_XTENSA_SLOT0_OP	.text.system_restart_core
       17:	f0c112               	addi	a1, a1, -16
       1a:	0109                	s32i.n	a0, a1, 0
       1c:	0228                	l32i.n	a2, a2, 0
       1e:	fffb01               	l32r	a0, c <system_restart_core-0x8>
     			1e: R_XTENSA_SLOT0_OP	.text.system_restart_core+0xc
     			1e: R_XTENSA_ASM_EXPAND	Wait_SPI_Idle
       21:	0000c0               	callx0	a0
       24:	fffb01               	l32r	a0, 10 <system_restart_core-0x4>
     			24: R_XTENSA_SLOT0_OP	.text.system_restart_core+0x10
     			24: R_XTENSA_ASM_EXPAND	Cache_Read_Disable
       27:	0000c0               	callx0	a0
       2a:	746c                	movi.n	a4, -25
       2c:	fff631               	l32r	a3, 4 <system_restart_core-0x10>
     			2c: R_XTENSA_SLOT0_OP	.text.system_restart_core+0x4
       2f:	0020c0               	memw
       32:	892322               	l32i	a2, a3, 0x224
       35:	102240               	and	a2, a2, a4
       38:	0020c0               	memw
       3b:	fff301               	l32r	a0, 8 <system_restart_core-0xc>
     			3b: R_XTENSA_SLOT0_OP	.text.system_restart_core+0x8
       3e:	896322               	s32i	a2, a3, 0x224
       41:	0000c0               	callx0	a0
       44:	0108                	l32i.n	a0, a1, 0
       46:	10c112               	addi	a1, a1, 16
       49:	f00d                	ret.n
     */

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
	static struct rst_info info = {REASON_DEFAULT_RST};
	return &info;
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
int ets_vprintf(void *routine, const char *format, va_list arg);

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
