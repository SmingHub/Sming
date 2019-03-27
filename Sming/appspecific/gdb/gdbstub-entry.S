/******************************************************************************
 * Copyright 2015 Espressif Systems
 *
 * Description: Assembly routines for the gdbstub
 *
 * License: ESPRESSIF MIT License
 *******************************************************************************/

#include "gdb/gdbstub-internal.h"

#include <xtensa/config/specreg.h>
#include <xtensa/config/core-isa.h>
#include <xtensa/corebits.h>

#define DEBUG_PC		(EPC + XCHAL_DEBUGLEVEL)
#define DEBUG_EXCSAVE	(EXCSAVE + XCHAL_DEBUGLEVEL)
#define DEBUG_PS		(EPS + XCHAL_DEBUGLEVEL)

.global gdbstub_savedRegs // GdbstubSavedRegisters

#if GDBSTUB_USE_OWN_STACK
.global gdbstub_exceptionStack
#endif

	ASATTR_GDBFN
.literal_position

	ASATTR_GDBINIT
.literal_position

	ASATTR_GDBFN
	.align	4

#ifdef ENABLE_GDB

/*
This is the debugging exception routine; it's called by the debugging vector

We arrive here with all regs intact except for a2. The old contents of A2 are saved
into the DEBUG_EXCSAVE special function register. EPC is the original PC.
*/
	.type gdbstub_debug_exception_entry, @function
gdbstub_debug_exception_entry:
/*
	//Minimum no-op debug exception handler, for debug
	rsr a2,DEBUG_PC
	addi a2,a2,3
	wsr a2,DEBUG_PC
	xsr	a2, DEBUG_EXCSAVE
	rfi	XCHAL_DEBUGLEVEL
*/

//Save all regs to structure
	movi	a2, gdbstub_savedRegs
	s32i	a0, a2, GDBSR_A(0)
	s32i	a1, a2, GDBSR_A(1)
	rsr		a0, DEBUG_PS
	s32i	a0, a2, GDBSR_ps
	rsr		a0, DEBUG_EXCSAVE //was R2
	s32i	a0, a2, GDBSR_A(2)
	s32i	a3, a2, GDBSR_A(3)
	s32i	a4, a2, GDBSR_A(4)
	s32i	a5, a2, GDBSR_A(5)
	s32i	a6, a2, GDBSR_A(6)
	s32i	a7, a2, GDBSR_A(7)
	s32i	a8, a2, GDBSR_A(8)
	s32i	a9, a2, GDBSR_A(9)
	s32i	a10, a2, GDBSR_A(10)
	s32i	a11, a2, GDBSR_A(11)
	s32i	a12, a2, GDBSR_A(12)
	s32i	a13, a2, GDBSR_A(13)
	s32i	a14, a2, GDBSR_A(14)
	s32i	a15, a2, GDBSR_A(15)
	rsr		a0, SAR
	s32i	a0, a2, GDBSR_sar
	rsr		a0, LITBASE
	s32i	a0, a2, GDBSR_litbase
	rsr		a0, 176
	s32i	a0, a2, GDBSR_sr176
	rsr		a0, 208
	s32i	a0, a2, GDBSR_sr208
	rsr		a0, DEBUGCAUSE
	s32i	a0, a2, GDBSR_cause
	rsr		a4, DEBUG_PC
	s32i	a4, a2, GDBSR_pc

#if GDBSTUB_USE_OWN_STACK
	//Move to our own stack
	movi a1, gdbstub_exceptionStack + (GDBSTUB_STACK_SIZE - 1) * 4
#endif

//If ICOUNT is -1, disable it by setting it to 0, otherwise we will keep triggering on the same instruction.
	rsr		a2, ICOUNT
	movi	a3, -1
	bne		a2, a3, noIcountReset
	movi	a3, 0
	wsr		a3, ICOUNT
noIcountReset:

	rsr	a2, ps
	addi	a2, a2, -PS_EXCM_MASK
	wsr	a2, ps
	rsync

//Call into the C code to do the actual handling.
	call0	gdbstub_handle_debug_exception

DebugExceptionExit:

	rsr	a2, ps
	addi	a2, a2, PS_EXCM_MASK
	wsr	a2, ps
	rsync

	//Restore registers from the gdbstub_savedRegs struct
	movi	a2, gdbstub_savedRegs
	l32i	a0, a2, GDBSR_pc
	wsr		a0, DEBUG_PC
//	l32i	a0, a2, 0x58
//	wsr		a0, 208
	l32i	a0, a2, GDBSR_sr176
	//wsr		a0, 176		//Some versions of gcc do not understand this...
	.byte  0x00, 176, 0x13	//so we hand-assemble the instruction.
	l32i	a0, a2, GDBSR_litbase
	wsr		a0, LITBASE
	l32i	a0, a2, GDBSR_sar
	wsr		a0, SAR
	l32i	a15, a2, GDBSR_A(15)
	l32i	a14, a2, GDBSR_A(14)
	l32i	a13, a2, GDBSR_A(13)
	l32i	a12, a2, GDBSR_A(12)
	l32i	a11, a2, GDBSR_A(11)
	l32i	a10, a2, GDBSR_A(10)
	l32i	a9, a2, GDBSR_A(8)
	l32i	a8, a2, GDBSR_A(8)
	l32i	a7, a2, GDBSR_A(7)
	l32i	a6, a2, GDBSR_A(6)
	l32i	a5, a2, GDBSR_A(5)
	l32i	a4, a2, GDBSR_A(4)
	l32i	a3, a2, GDBSR_A(3)
	l32i	a0, a2, GDBSR_A(2)
	wsr		a0, DEBUG_EXCSAVE //was R2
	l32i	a0, a2, GDBSR_ps
	wsr		a0, DEBUG_PS
	l32i	a1, a2, GDBSR_A(1)
	l32i	a0, a2, GDBSR_A(0)

	//Read back vector-saved a2 value, put back address of this routine.
	movi	a2, gdbstub_debug_exception_entry
	xsr	a2, DEBUG_EXCSAVE

	//All done. Return to where we came from.
	rfi	XCHAL_DEBUGLEVEL
	.size gdbstub_debug_exception_entry, .-gdbstub_debug_exception_entry


#endif // ENABLE_GDB


#ifdef HOOK_SYSTEM_EXCEPTIONS

	.global gdbstub_save_extra_sfrs_for_exception
	.type gdbstub_save_extra_sfrs_for_exception, @function
	ASATTR_GDBFN
	.align 4
//The Xtensa OS HAL does not save all the special function register things. This bit of assembly
//fills the gdbstub_savedRegs struct with them.
gdbstub_save_extra_sfrs_for_exception:
	movi	a2, gdbstub_savedRegs
	rsr		a3, LITBASE
	s32i	a3, a2, GDBSR_litbase
	rsr		a3, 176
	s32i	a3, a2, GDBSR_sr176
	rsr		a3, 208
	s32i	a3, a2, GDBSR_sr208
	rsr		a3, EXCCAUSE
	s32i	a3, a2, GDBSR_cause
	rsr		a3, EXCVADDR
	s32i	a3, a2, GDBSR_excvaddr
	ret
	.size gdbstub_save_extra_sfrs_for_exception, .-gdbstub_save_extra_sfrs_for_exception

#endif // HOOK_SYSTEM_EXCEPTIONS

#ifdef ENABLE_GDB

	.global gdbstub_init_debug_entry
	.global _DebugExceptionVector
	.type gdbstub_init_debug_entry, @function
	ASATTR_GDBINIT
	.align	4
gdbstub_init_debug_entry:
//This puts the following 2 instructions into the debug exception vector:
//	xsr	a2, DEBUG_EXCSAVE
//	jx	a2
	movi	a2, _DebugExceptionVector
	movi	a3, 0xa061d220
	s32i	a3, a2, 0
	movi	a3, 0x00000002
	s32i	a3, a2, 4

//Tell the just-installed debug vector where to go.
	movi	a2, gdbstub_debug_exception_entry
	wsr		a2, DEBUG_EXCSAVE

	ret
	.size gdbstub_init_debug_entry, .-gdbstub_init_debug_entry


//Set up ICOUNT register to step one single instruction
	.global gdbstub_icount_ena_single_step
	.type gdbstub_icount_ena_single_step, @function
	ASATTR_GDBFN
	.align 4
gdbstub_icount_ena_single_step:
	movi	a3, XCHAL_DEBUGLEVEL //Only count steps in non-debug mode
	movi	a2, -2
	wsr		a3, ICOUNTLEVEL
	wsr		a2, ICOUNT
	isync
	ret
	.size gdbstub_icount_ena_single_step, .-gdbstub_icount_ena_single_step


//These routines all assume only one breakpoint and watchpoint is available, which
//is the case for the ESP8266 Xtensa core.


	.global gdbstub_set_hw_breakpoint
	.type gdbstub_set_hw_breakpoint, @function
	ASATTR_GDBFN
gdbstub_set_hw_breakpoint:
	//a2 - addr, a3 - len (unused here)
	rsr		a4, IBREAKENABLE
	bbsi	a4, 0, return_w_error
	wsr		a2, IBREAKA
	movi	a2, 1
	wsr		a2, IBREAKENABLE
	isync
	movi 	a2, 1
	ret
	.size gdbstub_set_hw_breakpoint, .-gdbstub_set_hw_breakpoint

	.global gdbstub_del_hw_breakpoint
	.type gdbstub_del_hw_breakpoint, @function
	ASATTR_GDBFN
gdbstub_del_hw_breakpoint:
	//a2 - addr
	rsr		a5, IBREAKENABLE
	bbci	a5, 0, return_w_error
	rsr		a3, IBREAKA
	bne		a3, a2, return_w_error
	movi	a2,0
	wsr		a2, IBREAKENABLE
	isync
	movi	a2, 1
	ret
	.size gdbstub_del_hw_breakpoint, .-gdbstub_del_hw_breakpoint

	.global gdbstub_set_hw_watchpoint
	.type gdbstub_set_hw_watchpoint, @function
	ASATTR_GDBFN
	//a2 - addr, a3 - mask, a4 - type (1=read, 2=write, 3=access)
gdbstub_set_hw_watchpoint:
	//Check if any of the masked address bits are set. If so, that is an error.
	movi	a5,0x0000003F
	xor		a5, a5, a3
	bany	a2, a5, return_w_error
	//Check if watchpoint already is set
	rsr		a5, DBREAKC
	movi	a6, 0xC0000000
	bany	a6, a5, return_w_error
	//Set watchpoint
	wsr		a2, DBREAKA

	//Combine type and mask
	movi	a6, 0x3F
	and		a3, a3, a6
	slli	a4, a4, 30
	or		a3, a3, a4
	wsr		a3, DBREAKC

//	movi	a2, 1
	mov		a2, a3
	isync
	ret
	.size gdbstub_set_hw_watchpoint, .-gdbstub_set_hw_watchpoint


	.global gdbstub_del_hw_watchpoint
	.type gdbstub_del_hw_watchpoint, @function
	ASATTR_GDBFN
	//a2 - addr
gdbstub_del_hw_watchpoint:
	//See if the address matches
	rsr		a3, DBREAKA
	bne		a3, a2, return_w_error
	//See if the bp actually is set
	rsr		a3, DBREAKC
	movi	a2, 0xC0000000
	bnone	a3, a2, return_w_error
	//Disable bp
	movi	a2,0
	wsr		a2,DBREAKC
	movi	a2,1
	isync
	ret

return_w_error:
	movi	a2, 0
	ret
	.size gdbstub_del_hw_watchpoint, .-gdbstub_del_hw_watchpoint

#endif // ENABLE_GDB
