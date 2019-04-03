/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gdbsyscall.cpp - Support for GDB file I/O
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "gdbstub-cfg.h"

#if GDBSTUB_ENABLE_SYSCALL

#include "gdbsyscall.h"
#include "GdbPacket.h"
#include <gdb_syscall.h>
#include <sys/errno.h>
#include "Platform/System.h"

// Whilst GCC has intrinsics to do this, they eat IRAM
//#define bswap32(value) __builtin_bswap32(value)
//#define bswap64(value) __builtin_bswap64(value)

/** @brief Swap byte order in 32-bit value */
static uint32_t ATTR_GDBEXTERNFN bswap32(uint32_t value)
{
	return (value >> 24) | ((value >> 8) & 0xff00) | ((value << 8) & 0xff0000) | (value << 24);
}

/** @brief Swap byte order of array of 32-bit values
 *  @param words
 *  @param size
 */
static void ATTR_GDBEXTERNFN bswap32p(uint32_t* words, unsigned count)
{
	for(unsigned i = 0; i < count; ++i) {
		words[i] = bswap32(words[i]);
	}
}

/** @brief Swap word order in 64-bit value */
__forceinline void bswap64wp(uint64_t* value)
{
	*value = (*value >> 32) | (*value << 32);
}

static GdbSyscallInfo syscall_info; ///< The syscall packet

/*
 * If there is user data being packetised we need to wait until that's finished
 * before starting a syscall.
 *
 * If the call is asynchronous, we can set it up now and send it when user data has finished.
 * Otherwise we need to force send of uart data via sendUserData() (in gdbuart.cpp).
 *
 * Both of these deal with packetising whilst the debugger is attached but not paused.
 */
int gdb_syscall(const GdbSyscallInfo& info)
{
	if(!gdb_state.attached) {
		debug_e("syscall failed, not attached");
		return -EPERM;
	}
	if(gdb_state.syscall != syscall_ready) {
		debug_e("syscall failed, not ready");
		return -EAGAIN;
	}

	syscall_info = info;
	gdb_state.syscall = syscall_pending;

	debug_i("syscall pending");

	if(info.callback == nullptr) {
		// Wait for all user data to be output before sending request
		while(gdbstub_send_user_data() != 0) {
		}
		gdbstub_syscall_execute();
		// No callback, wait for completion
		while(gdb_state.syscall != syscall_ready) {
			// Watchdog will fire if this takes too long - intentional
		}
		debug_i("syscall returned %d", syscall_info.result);
		return syscall_info.result;
	} else {
		if(gdbstub_send_user_data() == 0) {
			gdbstub_syscall_execute();
		} else {
			// will be executed via user task in gdbuart
		}
		return 0;
	}
}

void gdbstub_syscall_execute()
{
	if(gdb_state.syscall != syscall_pending) {
		// Nothing to execute
		debug_w("No pending syscall");
		return;
	}

	auto& info = syscall_info;
	GdbPacket packet;
	packet.writeChar('F');
	switch(info.command) {
	case eGDBSYS_open:
		packet.writeStr(GDB_F("open,"));
		packet.writeStrRef(info.open.filename);
		packet.writeChar(',');
		packet.writeHexWord32(info.open.flags);
		packet.writeChar(',');
		packet.writeHexByte(0); // mode
		break;

	case eGDBSYS_close:
		packet.writeStr(GDB_F("close,"));
		packet.writeHexByte(info.close.fd);
		break;

	case eGDBSYS_read:
		packet.writeStr(GDB_F("read,"));
		packet.writeHexByte(info.read.fd);
		packet.writeChar(',');
		packet.writeHexWord32(uint32_t(info.read.buffer));
		packet.writeChar(',');
		packet.writeHexWord16(info.read.bufSize);
		break;

	case eGDBSYS_write:
		packet.writeStr(GDB_F("write,"));
		packet.writeHexByte(info.write.fd);
		packet.writeChar(',');
		packet.writeHexWord32(uint32_t(info.write.buffer));
		packet.writeChar(',');
		packet.writeHexWord16(info.write.count);
		break;

	case eGDBSYS_lseek:
		packet.writeStr(GDB_F("lseek,"));
		packet.writeHexByte(info.lseek.fd);
		packet.writeChar(',');
		packet.writeHexWord32(info.lseek.offset);
		packet.writeChar(',');
		packet.writeHexByte(info.lseek.whence);
		break;

	case eGDBSYS_rename:
		packet.writeStr(GDB_F("rename,"));
		packet.writeStrRef(info.rename.oldpath);
		packet.writeChar(',');
		packet.writeStrRef(info.rename.newpath);
		break;

	case eGDBSYS_unlink:
		packet.writeStr(GDB_F("unlink,"));
		packet.writeStrRef(info.unlink.pathname);
		break;

	case eGDBSYS_stat:
		packet.writeStr(GDB_F("stat,"));
		packet.writeStrRef(info.stat.pathname);
		packet.writeChar(',');
		packet.writeHexWord32(uint32_t(info.stat.buf));
		break;

	case eGDBSYS_fstat:
		packet.writeStr(GDB_F("fstat,"));
		packet.writeHexByte(info.fstat.fd);
		packet.writeChar(',');
		packet.writeHexWord32(uint32_t(info.fstat.buf));
		break;

	case eGDBSYS_gettimeofday:
		packet.writeStr(GDB_F("gettimeofday,"));
		packet.writeHexWord32(uint32_t(info.gettimeofday.tv));
		packet.writeChar(',');
		packet.writeHexWord32(uint32_t(info.gettimeofday.tz));
		break;

	case eGDBSYS_isatty:
		packet.writeStr(GDB_F("isatty,"));
		packet.writeHexByte(info.isatty.fd);
		break;

	case eGDBSYS_system:
		packet.writeStr(GDB_F("system,"));
		packet.writeStrRef(info.system.command);
		break;
	}

	// Discard incoming '+' acknolwedgement from packet
	++gdb_state.ack_count;
	gdb_state.syscall = syscall_active;

	debug_i("syscall active");
}

bool ATTR_GDBEXTERNFN gdb_syscall_complete(const char* data)
{
	// normally a hex value, but on error it's -1
	bool isNeg = false;
	if(*data == '-') {
		++data;
		isNeg = true;
	}
	int len = GdbPacket::readHexValue(data);
	if(isNeg) {
		len = -len;
	}

	char ctrl_c_flag = '\0';

	if(*data == ',') {
		++data;
		uint8_t err = GdbPacket::readHexValue(data);
		syscall_info.result = -err;
	} else {
		syscall_info.result = len;

		switch(syscall_info.command) {
		case eGDBSYS_gettimeofday: {
			auto tv = syscall_info.gettimeofday.tv;
			if(tv != nullptr) {
				// Content written in big-endian, so we need to correct it
				bswap32p(reinterpret_cast<uint32_t*>(tv), sizeof(gdb_timeval_t) / 4);
				bswap64wp(&tv->tv_usec);
			}
			break;
		}

		case eGDBSYS_stat:
		case eGDBSYS_fstat: {
			auto stat = syscall_info.stat.buf;
			if(stat != nullptr) {
				bswap32p(reinterpret_cast<uint32_t*>(stat), sizeof(gdb_stat_t) / 4);
				bswap64wp(&stat->st_size);
				bswap64wp(&stat->st_blksize);
				bswap64wp(&stat->st_blocks);
			}
			break;
		}

		default:; // No processing required for other commands
		}
	}

	if(*data == ',') {
		++data;
		ctrl_c_flag = *data;
	}

	if(syscall_info.callback != nullptr) {
		System.queueCallback(TaskCallback(syscall_info.callback), uint32_t(&syscall_info));
	}
	gdb_state.syscall = syscall_ready;

	return ctrl_c_flag;
}

#endif // GDBSTUB_ENABLE_SYSCALL
