

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
