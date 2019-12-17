void pp_soft_wdt_feed(void)
{
	//
}
/*
      6c:	00 00 00 00
			6c: R_XTENSA_32	.bss
      70:	00 06 00 60

00000074 <pp_soft_wdt_feed>:
      74:	fffe51               	l32r	a5, 6c <pp_soft_wdt_init+0x20>
			74: R_XTENSA_SLOT0_OP	.irom0.text+0x6c
      77:	040c                	movi.n	a4, 0
      79:	004542               	s8i	a4, a5, 0
      7c:	73a022               	movi	a2, 115
      7f:	fffc31               	l32r	a3, 70 <pp_soft_wdt_init+0x24>
			7f: R_XTENSA_SLOT0_OP	.irom0.text+0x70
      82:	0020c0               	memw
      85:	c56322               	s32i	a2, a3, 0x314
      88:	f00d                	ret.n
*/



void pp_soft_wdt_restart(void)
{
	//
}
/*
			d8: R_XTENSA_32	wDev_MacTim1SetFunc
			dc: R_XTENSA_32	wDev_MacTim1Arm

000000e0 <pp_soft_wdt_restart>:
      e0:	fffa21               	l32r	a2, c8 <pp_soft_wdt_stop+0x2c>
			e0: R_XTENSA_SLOT0_OP	.irom0.text+0xc8
      e3:	f0c112               	addi	a1, a1, -16
      e6:	0109                	s32i.n	a0, a1, 0
      e8:	fffc01               	l32r	a0, d8 <pp_soft_wdt_stop+0x3c>
			e8: R_XTENSA_SLOT0_OP	.irom0.text+0xd8
			e8: R_XTENSA_ASM_EXPAND	wDev_MacTim1SetFunc
      eb:	0000c0               	callx0	a0
      ee:	fff721               	l32r	a2, cc <pp_soft_wdt_stop+0x30>
			ee: R_XTENSA_SLOT0_OP	.irom0.text+0xcc
      f1:	0228                	l32i.n	a2, a2, 0
      f3:	fffa01               	l32r	a0, dc <pp_soft_wdt_stop+0x40>
			f3: R_XTENSA_SLOT0_OP	.irom0.text+0xdc
			f3: R_XTENSA_ASM_EXPAND	wDev_MacTim1Arm
      f6:	0000c0               	callx0	a0
      f9:	fff561               	l32r	a6, d0 <pp_soft_wdt_stop+0x34>
			f9: R_XTENSA_SLOT0_OP	.irom0.text+0xd0
      fc:	050c                	movi.n	a5, 0
      fe:	004652               	s8i	a5, a6, 0
     101:	73a032               	movi	a3, 115
     104:	fff441               	l32r	a4, d4 <pp_soft_wdt_stop+0x38>
			104: R_XTENSA_SLOT0_OP	.irom0.text+0xd4
     107:	0020c0               	memw
     10a:	c56432               	s32i	a3, a4, 0x314
     10d:	0108                	l32i.n	a0, a1, 0
     10f:	10c112               	addi	a1, a1, 16
     112:	f00d                	ret.n
*/


void pp_soft_wdt_stop(void)
{
	//
}

/*
      8c:	00 06 00 60
      90:	00 00 00 00
			90: R_XTENSA_32	.bss
      94:	80 1d 2c 04
      98:	00 00 00 00
			98: R_XTENSA_32	wDev_MacTim1Arm

0000009c <pp_soft_wdt_stop>:
      9c:	f0c112               	addi	a1, a1, -16
      9f:	0109                	s32i.n	a0, a1, 0
      a1:	73a032               	movi	a3, 115
      a4:	fffa41               	l32r	a4, 8c <pp_soft_wdt_feed+0x18>
			a4: R_XTENSA_SLOT0_OP	.irom0.text+0x8c
      a7:	0020c0               	memw
      aa:	fff921               	l32r	a2, 90 <pp_soft_wdt_feed+0x1c>
			aa: R_XTENSA_SLOT0_OP	.irom0.text+0x90
      ad:	000c                	movi.n	a0, 0
      af:	c56432               	s32i	a3, a4, 0x314
      b2:	004202               	s8i	a0, a2, 0
      b5:	fff721               	l32r	a2, 94 <pp_soft_wdt_feed+0x20>
			b5: R_XTENSA_SLOT0_OP	.irom0.text+0x94
      b8:	fff801               	l32r	a0, 98 <pp_soft_wdt_feed+0x24>
			b8: R_XTENSA_SLOT0_OP	.irom0.text+0x98
			b8: R_XTENSA_ASM_EXPAND	wDev_MacTim1Arm
      bb:	0000c0               	callx0	a0
      be:	0108                	l32i.n	a0, a1, 0
      c0:	10c112               	addi	a1, a1, 16
      c3:	f00d                	ret.n
 */
