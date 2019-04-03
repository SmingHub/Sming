/******************************************************************************
 * Copyright 2015 Espressif Systems
 *
 * Description: A stub to make the ESP8266 debuggable by GDB over the serial port.
 *
 * License: ESPRESSIF MIT License
 *
 * @author mikee47 <mike@sillyhouse.net>
 *
 * Sming doesn't require recompiling to work with GDB
 * GDB may be used for debug or release builds
 * If enabled, stub code is compiled into user application not into Sming library
 * Uses UART driver rather than accessing hardware directly.
 * GDB stub uses UART0 (tx/rx required) exclusively
 * Applications may use UART2, which is virtualised (using a generic callback mechanism) so
 * output may sent to debug console when GDB is attached.
 *
 * Note from GDB manual:
 *
 * 		At a minimum, a stub is required to support the ‘g’ and ‘G’ commands for register access,
 * 		and the ‘m’ and ‘M’ commands for memory access. Stubs that only control single-threaded
 * 		targets can implement run control with the ‘c’ (continue), and ‘s’ (step) commands. Stubs
 * 		that support multi-threading targets should support the ‘vCont’ command. All other commands
 * 		are optional.
 *
*********************************************************************************/

#include <xtensa/corebits.h>
#include <sys/errno.h>

#include "gdbstub.h"
#include "gdbstub-entry.h"
#include "gdb_hooks.h"
#include "GdbPacket.h"
#include "BitManipulations.h"
#include "exceptions.h"
#include "gdb/registers.h"
#include "gdbsyscall.h"
#include "Platform/System.h"

extern "C" {
void system_restart_core();
void Cache_Read_Enable_New();
};

/* Bit numbers for `cause` field for debug exceptions */
enum DebugCause {
	DBGCAUSE_ICOUNT = DEBUGCAUSE_ICOUNT_SHIFT,
	DBGCAUSE_IBREAK = DEBUGCAUSE_IBREAK_SHIFT,			  // Instruction breakpoint (hardware)
	DBGCAUSE_DBREAK = DEBUGCAUSE_DBREAK_SHIFT,			  // Data breakpoint (hardware Watchpoint)
	DBGCAUSE_BREAK = DEBUGCAUSE_BREAK_SHIFT,			  // Break instruction
	DBGCAUSE_BREAKN = DEBUGCAUSE_BREAKN_SHIFT,			  // Break.N instruction (16-bit form of BREAK)
	DBGCAUSE_DEBUG_INTERRUPT = DEBUGCAUSE_DEBUGINT_SHIFT, // Hardware debugger interrupt
};

// Register names
#define XX(num, name) static DEFINE_PSTR(xtreg_str_##name, #name)
XT_REGISTER_MAP(XX)
#undef XX
static PGM_P const registerNames[] PROGMEM = {
#define XX(num, name) xtreg_str_##name,
	XT_REGISTER_MAP(XX)
#undef XX
};

extern GdbstubSavedRegisters gdbstub_savedRegs;

typedef uint8_t ERRNO_T;

/*
 * Maximum length of a GDB command packet. Has to be at least able to fit the G command.
 */
#define MAX_COMMAND_LENGTH (16 + (GDB_REGCOUNT * 2 * 4))

#if GDBSTUB_USE_OWN_STACK
// This is the debugging exception stack - referred to in gdbstub-entry.S
uint32_t exceptionStack[GDBSTUB_STACK_SIZE];
#endif

volatile gdb_state_t gdb_state;					   ///< Global state
static char commandBuffer[MAX_COMMAND_LENGTH + 1]; ///< Buffer for incoming/outgoing GDB commands
static int32_t singleStepPs = -1; // Stores ps (Program State) when single-stepping instruction. -1 when not in use.

// Return codes from handleCommand()
enum GdbResult {
	ST_OK,	 ///< Regular command
	ST_CONT,   ///< Continue with program execution (but stay attached)
	ST_DETACH, ///< Detach from GDB and continue with program execution
};

// For simplifying access to word-aligned data
union WordStruct {
	uint32_t value;
	uint8_t bytes[4];
};

/*
 * @brief Read a byte from the ESP8266 memory
 * @param addr Address to read from
 * @retval uint8_t
 * @note No check is performed for `addr`, use validRdAddr() before calling this function
 */
static uint8_t ATTR_GDBEXTERNFN readMemoryByte(uint32_t addr)
{
	WordStruct word;
	word.value = *reinterpret_cast<uint32_t*>(addr & ~3);
	return word.bytes[addr & 3];
}

/*
 * @brief Write a byte to the ESP8266 memory
 * @param addr Address to write to
 * @param data
 * @note No check is performed for `addr`, use validWrAddr() before calling this function
 */
static void ATTR_GDBEXTERNFN writeMemoryByte(uint32_t addr, uint8_t data)
{
	auto valuePtr = reinterpret_cast<uint32_t*>(addr & ~3);
	WordStruct word;
	word.value = *valuePtr;
	word.bytes[addr & 3] = data;
	*valuePtr = word.value;
}

/*
 * Return true if addr lives within a given segment
 * See xtensa/config/core-isa.h for memory range information, XCHAL_* values
 */
static bool ATTR_GDBEXTERNFN inRange(uint32_t addr, uint32_t start, uint32_t size)
{
	return (addr >= start) && (addr < start + size);
}

#define IN_RANGE(addr, name) inRange(addr, XCHAL_##name##_VADDR, XCHAL_##name##_SIZE)

/*
 * Return true if it makes sense to write to addr
 */
static bool ATTR_GDBEXTERNFN isValidWriteAddr(uint32_t addr)
{
	return (addr >= 0x3ff00000 && addr < 0x40000000) || (addr >= 0x40100000 && addr < 0x40140000) ||
		   (addr >= 0x60000000 && addr < 0x60002000);
}

static bool ATTR_GDBEXTERNFN isValidWriteAddr(uint32_t startAddr, size_t length)
{
	return isValidWriteAddr(startAddr) && isValidWriteAddr(startAddr + length - 1);
}

/*
 * Return true if it makes sense to read from addr
 */
static bool ATTR_GDBEXTERNFN isValidReadAddr(uint32_t addr)
{
	// @todo Consider whether it's necessary to restrict read addresses at all
	return addr >= 0x20000000 && addr < 0x60000000;
}

static bool ATTR_GDBEXTERNFN isValidReadAddr(uint32_t startAddr, size_t length)
{
	return isValidReadAddr(startAddr) && isValidReadAddr(startAddr + length - 1);
}

/** @brief Write a block of memory
 *  @param addr
 *  @param length
 *  @retval uint8_t error number
 */
static ERRNO_T ATTR_GDBEXTERNFN writeMemoryBlock(uint32_t addr, const void* data, unsigned length)
{
	// write memory from gdb
	if(!isValidWriteAddr(addr, length)) {
		return EFAULT;
	}

	// If writing to Data RAM, use memcpy for efficiency
	if(IN_RANGE(addr, DATARAM0) || IN_RANGE(addr, DATARAM1)) {
		memcpy(reinterpret_cast<void*>(addr), data, length);
	} else {
		// Instruction RAM and registers must be read/written in 32-bit words
		for(unsigned i = 0; i < length; i++, addr++) {
			writeMemoryByte(addr, static_cast<const uint8_t*>(data)[i]);
		}
	}

	// Make sure caches are up-to-date. Procedure according to Xtensa ISA document, ISYNC inst desc.
	asm volatile("ISYNC\nISYNC\n");

	return 0;
}

/*
 * Send the reason execution is stopped to GDB.
 * The code sent is a standard GDB signal number. See signals.def.
 */
static void ATTR_GDBEXTERNFN sendReason()
{
	// Flush any debug info to serial
	while(gdbstub_send_user_data() != 0) {
		//
	}
	GdbPacket packet;
	packet.writeChar('T');

	uint8_t signal;
	auto cause = gdbstub_savedRegs.cause;
	if(bitRead(gdb_state.flags, DBGFLAG_SYSTEM_EXCEPTION)) {
		// Convert exception code to a signal number
		signal = (cause <= EXCCAUSE_MAX) ? pgm_read_byte(&gdb_exception_signals[cause]) : 0;
		packet.writeHexByte(signal);
	} else {
		// Debugging exception
		signal = bitRead(gdb_state.flags, DBGFLAG_CTRL_BREAK) ? GDB_SIGNAL_INT : GDB_SIGNAL_TRAP;
		packet.writeHexByte(signal);
// Current Xtensa GDB versions don't seem to request this, so let's leave it off.
#if 0
		if(bitRead(cause, 0)) {
			packet.writeStr(GDB_F("break")); // Single-step (ICOUNT hits 0)
		} else if(bitRead(cause, DBGCAUSE_IBREAK)) {
			packet.writeStr(GDB_F("hwbreak"));
		} else if(bitRead(cause, DBGCAUSE_DBREAK)) {
			packet.writeStr(GDB_F("watch:"));
			// ToDo: send address
		} else if(bitRead(cause, DBGCAUSE_BREAK)) {
			packet.writeStr(GDB_F("swbreak"));
		} else if(bitRead(cause, DBGCAUSE_BREAKN)) {
			packet.writeStr(GDB_F("swbreak"));
		}
#endif
	}
}

static void ATTR_GDBEXTERNFN sendOK()
{
	GdbPacket packet;
	packet.write(GDB_F("OK"), 2);
}

static void ATTR_GDBEXTERNFN sendError(uint8_t err)
{
	GdbPacket packet;
	packet.writeChar('E');
	packet.writeHexByte(err);
}

static void ATTR_GDBEXTERNFN sendReply(uint8_t err)
{
	if(err == 0) {
		sendOK();
	} else {
		sendError(err);
	}
}

static void ATTR_GDBEXTERNFN sendEmptyResponse()
{
	GdbPacket packet;
}

#if GDBSTUB_CMDENABLE_P
static uint32_t* ATTR_GDBEXTERNFN getSavedReg(unsigned regnum)
{
	switch(regnum) {
	case GdbReg_pc:
		return &gdbstub_savedRegs.pc;
	case GdbReg_sar:
		return &gdbstub_savedRegs.sar;
	case GdbReg_litbase:
		return &gdbstub_savedRegs.litbase;
	case GdbReg_sr176:
		return &gdbstub_savedRegs.sr176;
	case GdbReg_sr208:
		return &gdbstub_savedRegs.sr208;
	case GdbReg_ps:
		return &gdbstub_savedRegs.ps;
	default:
		regnum -= GdbReg_a0;
		if(regnum < 16) {
			return &gdbstub_savedRegs.a[regnum];
		} else {
			return nullptr;
		}
	}
}
#endif

/*
 * A command start character '$' has been detected, read the rest of the command packet.
 * @retval unsigned length of command, 0 on failure
 */
static unsigned ATTR_GDBEXTERNFN readCommand()
{
	uint8_t checksum = 0;
	size_t cmdLen = 0;
	int c;
	while((c = gdbReceiveChar()) != '#') { // end of packet, checksum follows
		if(c < 0) {
			gdbSendChar('-');
			debug_e("CMD TIMEOUT");
			return 0;
		}
		if(c == '$') {
			// Wut, restart packet?
			debug_e("Unexpected '$' received");
			checksum = 0;
			cmdLen = 0;
			continue;
		}
		checksum += uint8_t(c);
		if(c == '}') { // escape the next char
			c = gdbReceiveChar();
			checksum += uint8_t(c);
			c ^= 0x20;
		}
		if(cmdLen >= MAX_COMMAND_LENGTH) {
			// Received more than the size of the command buffer
			debug_e("Command '%c' buffer overflow", commandBuffer[0]);
			return 0;
		}
		commandBuffer[cmdLen++] = c;
	}
	commandBuffer[cmdLen] = '\0';

#if GDBSTUB_ENABLE_DEBUG
	debug_i("cmd '%c', len %u", commandBuffer[0], cmdLen);
#endif

	// Read checksum and verify
	char checksumChars[] = {char(gdbReceiveChar()), char(gdbReceiveChar()), '\0'};
	const char* ptr = checksumChars;
	auto receivedChecksum = GdbPacket::readHexValue(ptr);
	if(receivedChecksum == checksum) {
		// Acknowledge the command
		gdbSendChar('+');
	} else {
		// Command received, but checksum failed
		gdbSendChar('-');
		debug_e("Checksum mismatch, got 0x%02x, expected 0x%02x", receivedChecksum, checksum);
		cmdLen = 0;
	}
	return cmdLen;
}

/*
 * @brief Handle a command as received from GDB
 * @param cmdlen Number of characters in received command
 * @retval GdbResult One of ST_OK, ST_CONT or ST_DETACH
 */
static GdbResult ATTR_GDBEXTERNFN handleCommand(unsigned cmdLen)
{
	debug_hex(ERR, "CMD", commandBuffer, std::min(32U, cmdLen));

	uint8_t commandChar = commandBuffer[0];
	const char* data = &commandBuffer[1];
	switch(commandChar) {
	/*
	 * Send all registers to gdb
	 */
	case 'g': {
		// Lay register values out as expected by GDB
		memset(commandBuffer, 'x', GDB_REGCOUNT * 8); // All values undefined by default
		auto setreg = [&](unsigned index, uint32_t value) {
			GdbPacket::encodeHexBlock(&commandBuffer[index * 8], &value, sizeof(value));
		};
		for(int i = 0; i < 16; i++) {
			setreg(GdbReg_a0 + i, gdbstub_savedRegs.a[i]);
		}
		setreg(GdbReg_pc, gdbstub_savedRegs.pc);
		setreg(GdbReg_sar, gdbstub_savedRegs.sar);
		setreg(GdbReg_litbase, gdbstub_savedRegs.litbase);
		setreg(GdbReg_sr176, gdbstub_savedRegs.sr176);
		setreg(GdbReg_sr208, gdbstub_savedRegs.sr208);
		setreg(GdbReg_ps, gdbstub_savedRegs.ps);
		GdbPacket packet;
		packet.write(commandBuffer, GDB_REGCOUNT * 8);
		break;
	}

	/*
	 * Receive content for all registers from gdb
	 */
	case 'G': {
		// Decode hex values into array of words
		auto len = GdbPacket::decodeHexBlock(commandBuffer, data);
		if(len != GDB_REGCOUNT * 4) {
			debug_e("Invalid register block: got %u bytes, expected %u", len, GDB_REGCOUNT * 4);
			sendError(EINVAL);
			break;
		}
		// Store decoded values
		auto values = reinterpret_cast<uint32_t*>(commandBuffer);
		for(int i = 0; i < 16; i++) {
			gdbstub_savedRegs.a[i] = values[GdbReg_a0 + i];
		}
		gdbstub_savedRegs.pc = values[GdbReg_pc];
		gdbstub_savedRegs.sar = values[GdbReg_sar];
		gdbstub_savedRegs.litbase = values[GdbReg_litbase];
		gdbstub_savedRegs.sr176 = values[GdbReg_sr176];
		gdbstub_savedRegs.sr208 = values[GdbReg_sr208];
		gdbstub_savedRegs.ps = values[GdbReg_ps];
		sendOK();
		break;
	}

#if GDBSTUB_CMDENABLE_P
	/*
	 * Read/write an individual register
	 *
	 * Read: `p n'
	 * Write: `P n=r`
	 *
	 * n is the register number in hex
	 * r is hex-encoded value in target byte order
	 *
	 * For read command, return data is hex-encoded content in target byte order
	 */
	case 'p':
	case 'P': {
		auto regnum = GdbPacket::readHexValue(data);
		auto regPtr = getSavedReg(regnum);
#if GDBSTUB_ENABLE_DEBUG
		char regName[16];
		/*
 * @todo Trying to read flash here sometimes causes a LEVEL1 interrupt exception (even though
 * we haven't hooked it?!) According to the docs. we need to look at the INTERRUPT register
 * and INTENABLE to determine the actual cause.
 */
		regName[0] = '\0';
//		memcpy_P(regName, registerNames[regnum], sizeof(regName));
#endif

		if(commandChar == 'p') { // read
			GdbPacket packet;
#if GDBSTUB_ENABLE_DEBUG
			debug_i("GET %s", regName);
#endif
			if(regPtr == nullptr) {
				packet.writeX32();
			} else {
				packet.writeHexBlock(regPtr, sizeof(uint32_t));
			}
		} else {	// write
			data++; // skip =
			uint32_t value = 0;
			GdbPacket::decodeHexBlock(&value, data);
#if GDBSTUB_ENABLE_DEBUG
			debug_i("SET %s = 0x%08x", regName, value);
#endif
			if(regPtr == nullptr) {
				sendError(EINVAL);
			} else {
				*regPtr = value;
				sendOK();
			}
		}
		break;
	}
#endif // GDBSTUB_CMDENABLE_P

	/*
	 * read/write memory to gdb
	 *
	 * Read: `m addr,length`
	 * Write: `M addr,length:XX...`
	 *
	 * `addr` is address in hex, MSB first
	 * `length` is number of bytes, in hex, MSB first
	 */
	case 'm':
	case 'M': {
		uint32_t addr = GdbPacket::readHexValue(data);
		data++; // skip ,
		unsigned length = GdbPacket::readHexValue(data);

		if(commandChar == 'm') { // read memory to gdb
			if(isValidReadAddr(addr, length)) {
				GdbPacket packet;
				for(unsigned i = 0; i < length; i++) {
					packet.writeHexByte(readMemoryByte(addr));
					++addr;
				}
			} else {
				sendReply(EFAULT);
			}
			break;
		}

		ERRNO_T err = 0;

		// write memory from gdb
		data++; // skip :
		auto bin = reinterpret_cast<uint8_t*>(commandBuffer);
		auto len = GdbPacket::decodeHexBlock(bin, data);
		if(len == length) {
			err = writeMemoryBlock(addr, bin, length);
		} else {
			err = EINVAL;
		}

		sendReply(err);
		break;
	}

	/*
	 * Indicate the reason the target halted.
	 */
	case '?':
		sendReason();
		break;

	/*
	 * Continue execution
	 */
	case 'c':
		return ST_CONT;

	/*
	 * Single-step instruction
	 *
	 * Single-stepping can go wrong if an interrupt is pending, especially when it is e.g. a task switch:
	 * the ICOUNT register will overflow in the task switch code. That is why we disable interupts when
	 * doing single-instruction stepping.
	 */
	case 's':
		singleStepPs = gdbstub_savedRegs.ps;
		gdbstub_savedRegs.ps = (gdbstub_savedRegs.ps & ~0xf) | (XCHAL_DEBUGLEVEL - 1);
		gdbstub_icount_ena_single_step();
		return ST_CONT;

	/*
	 * Detach from debugger and continue with application execution
	 */
	case 'D':
		sendOK();
		return ST_DETACH;

#if GDBSTUB_ENABLE_SYSCALL
	/*
	 * A file (or console) I/O request has finished.
	 */
	case 'F':
		if(gdb_syscall_complete(data)) {
			// Ctrl+C was pressed
			sendReason();
			return ST_OK;
		} else {
			return ST_CONT;
		}
#endif

	/*
	 * Kill
	 *
	 * For a bare-metal target we restart the system. No reply is expected.
	 */
	case 'k':
		system_restart_core();
		break;

	/*
	 * Extended query
	 */
	case 'q': {
		GdbPacket packet;
		if(strncmp(data, GDB_F("Supported"), 9) == 0) { // Capabilities query
			packet.writeStr(GDB_F("swbreak+;hwbreak+;PacketSize="));
			packet.writeHexWord16(MAX_COMMAND_LENGTH);
		} else if(strncmp(data, GDB_F("Attached"), 8) == 0) {
			// Let gdb know that it is attaching to a running program
			// In general that just means it detaches instead of killing when it exits
			packet.writeChar('1');
		} else {
			// We don't support other queries, response will be empty
		}
		break;
	}

	/*
	 * break/watchpoint
	 *
	 * Remove: `z#,addr,kind`
	 * Insert: `Z#,addr,kind`
	 *
	 * Where # is:
	 * 	0 - software breakpoint (unsupported)
	 * 	1 - hardware breakpoint
	 * 	2 - write watchpoint
	 * 	3 - read watchpoint
	 * 	4 - access watchpoint
	 */
	case 'z':
	case 'Z': {
		char idx = *data;
		if(idx < '1' || idx > '4') {
			sendEmptyResponse();
			break;
		}

		uint8_t err = 0;
		data += 2; // skip 'x,'
		int addr = GdbPacket::readHexValue(data);
		data++; // skip ','
		int len = GdbPacket::readHexValue(data);
		if(commandChar == 'Z') { // Set hardware break/watchpoint
			if(idx == '1') {	 // breakpoint
				if(!gdbstub_set_hw_breakpoint(addr, len)) {
					err = EPERM;
				}
			} else { // watchpoint
				int access;
				unsigned int mask = 0;
				if(idx == '2')
					access = 2; // write
				else if(idx == '3')
					access = 1; // read
				else if(idx == '4')
					access = 3; // access
				if(len == 1)
					mask = 0x3F;
				else if(len == 2)
					mask = 0x3E;
				else if(len == 4)
					mask = 0x3C;
				else if(len == 8)
					mask = 0x38;
				else if(len == 16)
					mask = 0x30;
				else if(len == 32)
					mask = 0x20;
				else
					err = EINVAL;

				if(err == 0 && !gdbstub_set_hw_watchpoint(addr, mask, access)) {
					err = EPERM;
				}
			}
		} else { // Clear hardware break/watchpoint
			bool ok;
			if(idx == '1') { // breakpoint
				ok = gdbstub_del_hw_breakpoint(addr);
			} else { // watchpoint
				ok = gdbstub_del_hw_watchpoint(addr);
			}
			if(!ok) {
				err = EPERM;
			}
		}
		sendReply(err);
		break;
	}

#ifdef GDBSTUB_CMDENABLE_V
	/*
	 * Packet starting with 'v' are identified by a multi-letter name, up to the first ':' or '?' (or end of packet)
	 */
	case 'v':
		/*
		 * `vFile:operation:parameter...`
		 *
		 * @todo Consider whether this might be useful
		 */
		if(strncmp(data, GDB_F("File:"), 5) == 0) {
			data += 5;
			if(strncmp(data, GDB_F("open:"), 5) == 0) {
				data += 5;
				char* filename;
				size_t len = GdbPacket::decodeHexBlock(data, filename);
				filename[len] = '\0';
				debug_i("File:open('%s')", filename);
				sendOK();
				break;
			}
		}
		/*
		 * vFlashErase:addr,length
		 */
		else if(strncmp(data, GDB_F("FlashErase:"), 11) == 0) {
			data += 11;
			// @todo
		}
		/*
		 * vFlashWrite:addr:XX...
		 */
		else if(strncmp(data, GDB_F("FlashWrite:"), 11) == 0) {
			data += 11;
			// @todo
		}
		/*
		 * vFlashDone
		 *
		 * Indicates that programming operation is finished
		 */
		else if(strncmp(data, GDB_F("FlashDone:"), 10) == 0) {
			data += 10;
			// @todo
		}
		sendEmptyResponse();
		break;
#endif

#ifdef GDBSTUB_CMDENABLE_X
	/*
	 * Write data to memory, where the data is transmitted in binary,
	 * which is more efficient than hex packets (see 'M').
	 *
	 * `X addr,length:XX...`
	 *
	 */
	case 'X': {
		uint32_t addr = GdbPacket::readHexValue(data);
		data++; // skip ,
		unsigned length = GdbPacket::readHexValue(data);
		data++; // skip :
		uint8_t err = writeMemoryBlock(addr, data, length);
		sendReply(err);
		break;
	}
#endif

	default:
		// We don't recognize or support whatever GDB just sent us.
		sendEmptyResponse();
	}
	return ST_OK;
}

/**
 * @brief Wait for incoming commands and process them, only returning when instructed to do so by GDB
 * @param waitForStart false if start character '$' has already been received
 * @param allowDetach When handling exceptions we ignore detach requests
 * @retval GdbResult cause of command loop exit
 * @note Flags that gdb has been attached whenever a gdb-formatted packet is received
 * Keeps reading commands until either a continue, detach, or kill command is received
 * It is not necessary for gdb to be attached for it to be paused
 * For example, during an exception break, the program is paused but gdb might not be attached yet
*/
void ATTR_GDBEXTERNFN commandLoop(bool waitForStart, bool allowDetach)
{
	bool initiallyAttached = gdb_state.attached;

	while(true) {
		if(waitForStart) {
			while(gdbReceiveChar() != '$') {
				// wait for start
			}
		}
		waitForStart = true;

		auto cmdLen = readCommand();
		if(cmdLen != 0) {
			gdb_state.attached = true;
			GdbResult result = handleCommand(cmdLen);
			if(result == ST_CONT) {
				break;
			} else if(result == ST_DETACH && allowDetach) {
				gdb_state.attached = false;
				break;
			}
		}
	}

	if(gdb_state.attached != initiallyAttached) {
		System.queueCallback(TaskCallback(gdb_on_attach), gdb_state.attached);
	}
}

/*
 * Emulate the l32i/s32i instruction we're stopped at
 */
static void ATTR_GDBEXTERNFN emulLdSt()
{
	// Lower 16 bits of instruction
	union {
		uint8_t b[2];
		struct {
			uint16_t op0 : 4; // Opcode
			uint16_t t : 4;   // Target 'A' register
			uint16_t s : 4;   // Source 'A' register
			uint16_t r : 4;   // Instruction dependent
		};
	} inst = {readMemoryByte(gdbstub_savedRegs.pc), readMemoryByte(gdbstub_savedRegs.pc + 1)};

#if GDBSTUB_ENABLE_DEBUG >= 3
	debug_i("emulLdSt(%u, %u, %u, %u)", inst.op0, inst.t, inst.s, inst.r);
#endif

	auto& reg = gdbstub_savedRegs;
	if(inst.op0 == 2 && (inst.r & 0x0b) == 2) {
		// l32i or s32i
		uint8_t imm8 = readMemoryByte(reg.pc + 2);
		auto p = reinterpret_cast<uint32_t*>(reg.a[inst.s]) + imm8; // offset in words
		if(inst.r == 2) {											// l32i
			reg.a[inst.t] = *p;
		} else { // s32i
			*p = reg.a[inst.t];
		}
		reg.pc += 3;
	} else if((inst.op0 & 0xe) == 8) {
		// l32i.n or s32i.n
		auto p = reinterpret_cast<uint32_t*>(reg.a[inst.s]) + inst.r; // offset in words
		if(inst.op0 == 8) {											  // l32i.n
#if GDBSTUB_ENABLE_DEBUG >= 3
			debug_i("l32i.n A%u, A%u, %u (0x%08x)", inst.t, inst.s, inst.r, *p);
#endif
			reg.a[inst.t] = *p;
		} else {
#if GDBSTUB_ENABLE_DEBUG >= 3
			debug_i("s32i.n A%u, A%u, %u (0x%08x)", inst.t, inst.s, inst.r, reg.a[inst.t]);
#endif
			*p = reg.a[inst.t];
		}
		reg.pc += 2;
	} else {
		debug_e("GDBSTUB: No l32i/s32i instruction: (%u, %u, %u, %u)", inst.op0, inst.t, inst.s, inst.r);
	}
}

/**
 * @brief If the hardware timer is operating using non-maskable interrupts, we must explicitly stop it
 * @param pause true to stop the timer, false to resume it
 */
static void pauseHardwareTimer(bool pause)
{
#if GDBSTUB_PAUSE_HARDWARE_TIMER
	static bool edgeIntEnable;
	if(pause) {
		edgeIntEnable = bitRead(READ_PERI_REG(EDGE_INT_ENABLE_REG), 1);
		TM1_EDGE_INT_DISABLE();
	} else if(edgeIntEnable) {
		TM1_EDGE_INT_ENABLE();
	}
#endif
}

// Main exception handler
static void __attribute__((noinline)) gdbstub_handle_debug_exception_flash()
{
	bool isEnabled = gdb_state.enabled;

	if(isEnabled) {
		pauseHardwareTimer(true);

		bitSet(gdb_state.flags, DBGFLAG_DEBUG_EXCEPTION);

		if(singleStepPs >= 0) {
			// We come here after single-stepping an instruction
			// Interrupts are disabled for the single step: re-enable them here
			gdbstub_savedRegs.ps = (gdbstub_savedRegs.ps & ~0xf) | (singleStepPs & 0xf);
			singleStepPs = -1;
		}

		if(bitRead(gdb_state.flags, DBGFLAG_PACKET_STARTED)) {
			commandLoop(false, true);
		} else {
			sendReason();
			commandLoop(true, true);
		}
	}

	// Watchpoint ?
	if(bitRead(gdbstub_savedRegs.cause, DBGCAUSE_DBREAK)) {
		// We stopped due to a watchpoint. We can't re-execute the current instruction
		// because it will happily re-trigger the same watchpoint, so we emulate it
		// while we're still in debugger space.
		emulLdSt();
	}

	// BREAK ?
	else if(bitRead(gdbstub_savedRegs.cause, DBGCAUSE_BREAK)) {
		// Skip instruction, first confirming it's actually a BREAK as GDB may have replaced it with
		// the original instruction if it's one of the breakpoints it set.
		if(readMemoryByte(gdbstub_savedRegs.pc + 2) == 0 && (readMemoryByte(gdbstub_savedRegs.pc + 1) & 0xf0) == 0x40 &&
		   (readMemoryByte(gdbstub_savedRegs.pc) & 0x0f) == 0x00) {
			gdbstub_savedRegs.pc += 3;
		}
	}

	// BREAK.N ?
	else if(bitRead(gdbstub_savedRegs.cause, DBGCAUSE_BREAKN)) {
		// Skip instruction, first confirming it is actually a BREAK.N
		if((readMemoryByte(gdbstub_savedRegs.pc + 1) & 0xf0) == 0xf0 && readMemoryByte(gdbstub_savedRegs.pc) == 0x2d) {
			gdbstub_savedRegs.pc += 3;
		}
	}

	gdb_state.flags = 0;

	if(isEnabled) {
		pauseHardwareTimer(false);
	}
}

// We just caught a debug exception and need to handle it. This is called from an assembly routine in gdbstub-entry.S
extern "C" void IRAM_ATTR gdbstub_handle_debug_exception()
{
	Cache_Read_Enable_New();
	gdbstub_handle_debug_exception_flash();
}

#if GDBSTUB_BREAK_ON_EXCEPTION
void gdbstub_handle_exception()
{
	if(!gdb_state.enabled) {
		return;
	}

	pauseHardwareTimer(true);
	bitSet(gdb_state.flags, DBGFLAG_SYSTEM_EXCEPTION);

	sendReason();
	commandLoop(true, false);

	gdb_state.flags = 0;
	pauseHardwareTimer(false);
}
#endif

// gdb_init() calls this after any exception handler initialisation
void ATTR_GDBINIT gdbstub_init()
{
	gdbstub_init_debug_entry();

	gdb_state.enabled = gdb_uart_init();

#define SD(xx) debug_i(#xx " = %u", xx)
	SD(ENABLE_EXCEPTION_DUMP);
	SD(ENABLE_CRASH_DUMP);
	SD(GDBSTUB_ENABLE_DEBUG);
	SD(GDBSTUB_PAUSE_HARDWARE_TIMER);
	SD(GDBSTUB_GDB_PATCHED);
	SD(GDBSTUB_USE_OWN_STACK);
	SD(GDBSTUB_BREAK_ON_EXCEPTION);
	SD(GDBSTUB_CTRLC_BREAK);
	SD(GDBSTUB_BREAK_ON_INIT);
	SD(GDBSTUB_ENABLE_UART2);
	SD(GDBSTUB_ENABLE_SYSCALL);
#undef SD

#if GDBSTUB_BREAK_ON_INIT
	if(gdb_state.enabled) {
		gdbstub_do_break();
	}
#endif
}

/* Hook functions */

GdbState IRAM_ATTR gdb_present()
{
	return gdb_state.attached ? eGDB_Attached : eGDB_Detached;
}

void gdb_enable(bool state)
{
	gdb_state.enabled = state;
}

void IRAM_ATTR gdb_do_break()
{
	gdbstub_do_break();
}
