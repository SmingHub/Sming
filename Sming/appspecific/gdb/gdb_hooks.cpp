/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gdb_hooks.cpp
 *
 ****/

#include <gdb_hooks.h>
#include <gdb_syscall.h>
#include "gdb/gdbstub.h"
#include "gdb/gdbstub-entry.h"
#include "gdb/exceptions.h"

extern "C" {

void Cache_Read_Enable_New();

typedef void (*xtos_handler_func)(UserFrame* frame);
void _xtos_set_exception_handler(int, xtos_handler_func);
}

// List of GDB signals used for exceptions
const uint8_t gdb_exception_signals[] GDB_PROGMEM = {
#define XX(ex, sig, desc) sig,
	SYSTEM_EXCEPTION_MAP(XX)
#undef XX
};

// List of exception names
#define XX(ex, sig, desc) static DEFINE_PSTR(exception_str_##ex, #ex)
SYSTEM_EXCEPTION_MAP(XX)
#undef XX
static PGM_P const exceptionNames[] PROGMEM = {
#define XX(ex, sig, desc) exception_str_##ex,
	SYSTEM_EXCEPTION_MAP(XX)
#undef XX
};

// The asm stub saves the Xtensa registers here when an exception is raised
GdbstubSavedRegisters gdbstub_savedRegs;

void debug_print_stack(uint32_t start, uint32_t end)
{
	m_puts(_F("\nStack dump:\n"));
	PSTR_ARRAY(instructions, "To decode the stack dump call from command line:\n"
							 "   python $SMING_HOME/../tools/decode-stacktrace.py out/build/app.out\n"
							 "and copy & paste the text enclosed in '===='.\n");
	PSTR_ARRAY(separatorLine, "\n================================================================\n");
	m_puts(instructions);
	m_puts(separatorLine);
	for(uint32_t addr = start; addr < end; addr += 0x10) {
		uint32_t* values = (uint32_t*)addr;
		// rough indicator: stack frames usually have SP saved as the second word
		bool looksLikeStackFrame = (values[2] == addr + 0x10);

		m_printf(_F("%08x:  %08x %08x %08x %08x %c\n"), addr, values[0], values[1], values[2], values[3],
				 (looksLikeStackFrame) ? '<' : ' ');
	}
	m_puts(separatorLine);
	m_puts(instructions);
}

void debug_crash_callback(const rst_info* rst_info, uint32_t stack, uint32_t stack_end)
{
#if ENABLE_CRASH_DUMP
	switch(rst_info->reason) {
	case REASON_EXCEPTION_RST:
		m_printf(_F("\n\n***** Exception Reset (%u):\n"
					"epc1=0x%08x epc2=0x%08x epc3=0x%08x excvaddr=0x%08x depc=0x%08x\n"),
				 rst_info->exccause, rst_info->epc1, rst_info->epc2, rst_info->epc3, rst_info->excvaddr,
				 rst_info->depc);
		break;
	case REASON_SOFT_WDT_RST:
		m_puts(_F("\n\n***** Software Watchdog Reset\n"));
		break;
	default:
		//
		;
	}

	debug_print_stack(stack, stack_end);
#endif
}

#if ENABLE_EXCEPTION_DUMP

void dumpExceptionInfo(UserFrame* frame)
{
	ets_wdt_disable();
	auto& reg = gdbstub_savedRegs;

	m_printf(_F("\n\n***** Fatal exception %u"), reg.cause);
	if(reg.cause <= EXCCAUSE_MAX) {
		m_putc(' ');
		m_putc('(');
		char name[32];
		memcpy_P(name, exceptionNames[reg.cause], sizeof(name));
		name[sizeof(name) - 1] = '\0';
		m_puts(name);
		m_putc(')');
	}
	m_putc('\n');

	// EXCVADDR isn't set for all exceptions, so zero it out rather than show potentially misleading information
	if(reg.cause < EXCCAUSE_UNALIGNED && reg.cause != EXCCAUSE_IFETCHERROR && reg.cause != EXCCAUSE_LOAD_STORE_ERROR) {
		reg.excvaddr = 0;
	}

	m_printf(_F("pc=0x%08x sp=0x%08x excvaddr=0x%08x\n"), reg.pc, reg.a[1], reg.excvaddr);
	m_printf(_F("ps=0x%08x sar=0x%08x vpri=0x%08x\n"), reg.ps, reg.sar, reg.vpri);
	for(int i = 0; i < 16; i++) {
		uint32_t r = reg.a[i];
		m_printf(_F("r%02u: 0x%08x=%10d "), i, r, r);
		if(i % 3 == 2) {
			m_putc('\n');
		}
	}
	m_putc('\n');
	debug_print_stack(reg.a[1], 0x3fffffb0);
	ets_wdt_enable();
}
#endif

#ifdef HOOK_SYSTEM_EXCEPTIONS

// Main exception handler code
static void __attribute__((noinline)) gdbstub_exception_handler_flash(UserFrame* frame)
{
	// Copy registers the Xtensa HAL did save to gdbstub_savedRegs
	memcpy(&gdbstub_savedRegs, frame, 5 * 4);
	memcpy(&gdbstub_savedRegs.a[2], &frame->a2, 14 * 4);
	// Credits go to Cesanta for this trick. A1 seems to be destroyed, but because it
	// has a fixed offset from the address of the passed frame, we can recover it.
	const uint32_t EXCEPTION_GDB_SP_OFFSET = 0x100;
	gdbstub_savedRegs.a[1] = uint32_t(frame) + EXCEPTION_GDB_SP_OFFSET;

#if ENABLE_EXCEPTION_DUMP
	if(gdb_present() != eGDB_Attached) {
		dumpExceptionInfo(frame);
	}
#endif

#if defined(ENABLE_GDB) && GDBSTUB_BREAK_ON_EXCEPTION
	gdbstub_handle_exception();

	// Copy any changed registers back to the frame the Xtensa HAL uses.
	memcpy(frame, &gdbstub_savedRegs, 5 * 4);
	memcpy(&frame->a2, &gdbstub_savedRegs.a[2], 14 * 4);

	return;
#endif

	// Wait for watchdog to reset system
	while(true)
		;
}

// Non-OS exception handler. Gets called by the Xtensa HAL.
static void IRAM_ATTR gdbstub_exception_handler(UserFrame* frame)
{
	// Save the extra registers the Xtensa HAL doesn't save
	gdbstub_save_extra_sfrs_for_exception();
	Cache_Read_Enable_New();
	gdbstub_exception_handler_flash(frame);
}

// The OS-less SDK uses the Xtensa HAL to handle exceptions. We can use those functions to catch any
// fatal exceptions and invoke the debugger when this happens.
static void ATTR_GDBINIT installExceptionHandler()
{
	for(unsigned ex = 0; ex <= EXCCAUSE_MAX; ++ex) {
		unsigned signal = pgm_read_byte(&gdb_exception_signals[ex]);
		if(signal != 0) {
			_xtos_set_exception_handler(ex, gdbstub_exception_handler);
		}
	}
}

#endif // HOOK_SYSTEM_EXCEPTIONS

void ATTR_GDBINIT gdb_init()
{
#ifdef HOOK_SYSTEM_EXCEPTIONS
	installExceptionHandler();
#endif

#ifdef ENABLE_GDB
	gdbstub_init();
#endif
}

extern "C" {
static unsigned IRAM_ATTR __gdb_no_op()
{
	return 0;
}

#define NOOP __attribute__((weak, alias("__gdb_no_op")))

void gdb_enable(bool state) NOOP;
void gdb_do_break(void) NOOP;
GdbState gdb_present(void) NOOP;
void gdb_on_attach(bool attached) NOOP;
};

int __attribute__((weak)) gdb_syscall(const GdbSyscallInfo& info)
{
	return -1;
}
