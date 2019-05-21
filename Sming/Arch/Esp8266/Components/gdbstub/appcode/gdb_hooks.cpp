/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gdb_hooks.cpp
 *
 ****/

#include <gdb/gdb_hooks.h>
#include <gdb/gdb_syscall.h>
#include "gdbstub/gdbstub.h"
#include "gdbstub/gdbuart.h"
#include "gdbstub/gdbstub-entry.h"
#include "gdbstub/exceptions.h"
#include <driver/uart.h>

extern "C" void Cache_Read_Enable_New();

// List of GDB signals used for exceptions
const uint8_t gdb_exception_signals[] GDB_PROGMEM = {
#define XX(ex, sig, desc) sig,
	SYSTEM_EXCEPTION_MAP(XX)
#undef XX
};

// List of exception names
#define XX(ex, sig, desc) DEFINE_PSTR_LOCAL(exception_str_##ex, #ex)
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
	m_puts(_F("\r\n"
			  "Stack dump:\r\n"));
	PSTR_ARRAY(instructions, "To decode the stack dump call from command line:\r\n"
							 "   make decode-stacktrace\r\n"
							 "and copy & paste the text enclosed in '===='.\r\n");
	PSTR_ARRAY(separatorLine, "\n================================================================\r\n");
	m_puts(instructions);
	m_puts(separatorLine);
	for(uint32_t addr = start; addr < end; addr += 0x10) {
		uint32_t* values = (uint32_t*)addr;
		// rough indicator: stack frames usually have SP saved as the second word
		bool looksLikeStackFrame = (values[2] == addr + 0x10);

		m_printf(_F("%08x:  %08x %08x %08x %08x %c\r\n"), addr, values[0], values[1], values[2], values[3],
				 (looksLikeStackFrame) ? '<' : ' ');

		system_soft_wdt_feed();
		wdt_feed();
	}
	m_puts(separatorLine);
	m_puts(instructions);
}

void debug_crash_callback(const rst_info* rst_info, uint32_t stack, uint32_t stack_end)
{
#ifdef ENABLE_GDB
	gdbFlushUserData();
	if(gdb_state.attached) {
		m_setPuts(gdbWriteConsole);
	}
#endif

#if defined(ENABLE_GDB) || ENABLE_CRASH_DUMP
	switch(rst_info->reason) {
	case REASON_EXCEPTION_RST:
		m_printf(_F("\r\n"
					"\n"
					"***** Exception Reset (%u):\r\n"
					"epc1=0x%08x epc2=0x%08x epc3=0x%08x excvaddr=0x%08x depc=0x%08x\r\n"),
				 rst_info->exccause, rst_info->epc1, rst_info->epc2, rst_info->epc3, rst_info->excvaddr,
				 rst_info->depc);
		break;
	case REASON_SOFT_WDT_RST:
		m_puts(_F("\r\n"
				  "\n"
				  "***** Software Watchdog Reset\r\n"));
		break;
	default:
		//
		;
	}

#if defined(ENABLE_GDB) && GDBSTUB_BREAK_ON_RESTART
	// Drop interrupt level to enable break instruction
	__asm__("rsil a2, 1"); // XCHAL_DEBUGLEVEL - 1
	gdbstub_break_internal(DBGFLAG_RESTART);
#elif ENABLE_CRASH_DUMP
	debug_print_stack(stack, stack_end);
#endif

#endif // defined(ENABLE_GDB) || ENABLE_CRASH_DUMP
}

#ifdef HOOK_SYSTEM_EXCEPTIONS

void dumpExceptionInfo()
{
	auto& reg = gdbstub_savedRegs;

	m_printf(_F("\r\n"
				"\n"
				"***** Fatal exception %u"),
			 reg.cause);
	if(reg.cause <= EXCCAUSE_MAX) {
		m_printf(_F(" (%s)"), exceptionNames[reg.cause]);
	}
	m_puts("\r\n");

	// EXCVADDR isn't set for all exceptions, so zero it out rather than show potentially misleading information
	if(reg.cause < EXCCAUSE_UNALIGNED && reg.cause != EXCCAUSE_IFETCHERROR && reg.cause != EXCCAUSE_LOAD_STORE_ERROR) {
		reg.excvaddr = 0;
	}

	m_printf(_F("pc=0x%08x sp=0x%08x excvaddr=0x%08x\r\n"), reg.pc, reg.a[1], reg.excvaddr);
	m_printf(_F("ps=0x%08x sar=0x%08x vpri=0x%08x\r\n"), reg.ps, reg.sar, reg.vpri);
	for(int i = 0; i < 16; i++) {
		uint32_t r = reg.a[i];
		m_printf(_F("r%02u: 0x%08x=%10d "), i, r, r);
		if(i % 3 == 2) {
			m_puts("\r\n");
		}
	}
	m_puts("\r\n");
	// Stack dump can be quite large, and not helpful to dump it to GDB console
	if(gdb_present() != eGDB_Attached) {
		debug_print_stack(reg.a[1], 0x3fffffb0);
	}
}

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

#if defined(ENABLE_GDB) && GDBSTUB_BREAK_ON_EXCEPTION
	gdbFlushUserData();

	// If GDB is attached, temporarily redirect m_printf calls to the console
	nputs_callback_t oldPuts = nullptr;
	if(gdb_state.attached) {
		oldPuts = m_setPuts(gdbWriteConsole);
	}

	dumpExceptionInfo();

	if(gdb_state.attached) {
		m_setPuts(oldPuts);
	}

	gdbstub_handle_exception();

	// Copy any changed registers back to the frame the Xtensa HAL uses.
	memcpy(frame, &gdbstub_savedRegs, 5 * 4);
	memcpy(&frame->a2, &gdbstub_savedRegs.a[2], 14 * 4);

#else

	dumpExceptionInfo();

#if defined(ENABLE_GDB)
	gdbFlushUserData();
#endif

	// Wait for watchdog to reset system
	while(true)
		;
#endif
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
			_xtos_set_exception_handler(ex, (_xtos_handler)gdbstub_exception_handler);
		}
	}
}

#endif // HOOK_SYSTEM_EXCEPTIONS

extern "C" {

void ATTR_GDBINIT gdb_init(void)
{
	// Reset all serial ports to user-specified baud rate
	for(unsigned i = 0; i < UART_PHYSICAL_COUNT; ++i) {
		uart_set_baudrate_reg(i, SERIAL_BAUD_RATE);
	}

#ifdef HOOK_SYSTEM_EXCEPTIONS
	installExceptionHandler();
#endif

#ifdef ENABLE_GDB
	gdbstub_init();
#endif
}

static unsigned IRAM_ATTR __gdb_no_op(void)
{
	return 0;
}

#define NOOP __attribute__((weak, alias("__gdb_no_op")))

void gdb_enable(bool state) NOOP;
GdbState gdb_present(void) NOOP;
void gdb_on_attach(bool attached) NOOP;
void gdb_detach(void) NOOP;
};

int WEAK_ATTR gdb_syscall(const GdbSyscallInfo& info)
{
	return -1;
}
