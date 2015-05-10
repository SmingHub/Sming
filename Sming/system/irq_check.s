	.data
.Lfmt:
.asciz	">>> 0x%08x -> 0x%08x: called from IRQ (or with interrupts disabled)\n"
	.text
	.global _mcount
	.align	4
	.literal_position
	.align	4
_mcount:
	movi	a11, 0x40200000
	bgeu	a0, a11, 1f
	ret
1:
	rsr	a11, ps
	extui	a11, a11, 0, 4
	bnez	a11, 1f
	ret
1:
	addi	a1, a1, -32
	s32i	a0, a1, 0
	s32i	a2, a1, 8
	s32i	a3, a1, 12
	s32i	a4, a1, 16
	s32i	a5, a1, 20
	s32i	a6, a1, 24
	s32i	a7, a1, 28
	s32i	a8, a1, 4

	movi	a2, .Lfmt
	mov	a3, a10
	mov	a4, a0
	call0	ets_printf

	l32i	a0, a1, 0
	l32i	a2, a1, 8
	l32i	a3, a1, 12
	l32i	a4, a1, 16
	l32i	a5, a1, 20
	l32i	a6, a1, 24
	l32i	a7, a1, 28
	l32i	a8, a1, 4
	addi	a1, a1, 32
	ret

