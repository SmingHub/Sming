/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gdb_syscall.h - Support for GDB file I/O
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 * An API is defined for communicating with GDB using the file-I/O protocol, but can also perform
 * functions not related to file (or console) I/O.
 *
 * This is a synchronous protocol, so only one request may be in progress at a time.
 *
 * To use in your application, build with GDBSTUB_ENABLE_SYSCALL=1 and #include this header.
 *
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/File_002dI_002fO-Remote-Protocol-Extension.html
 *
 ****/

#ifndef _SYSTEM_INCLUDE_GDB_SYSCALL_H_
#define _SYSTEM_INCLUDE_GDB_SYSCALL_H_

#include <user_config.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

/* GDB uses a specific version of the stat structure, 64 bytes in size */
struct __attribute__((packed)) gdb_stat_t {
	uint32_t st_dev;	 // device
	uint32_t st_ino;	 // inode
	mode_t st_mode;		 // protection
	uint32_t st_nlink;   // number of hard links
	uint32_t st_uid;	 // user ID of owner
	uint32_t st_gid;	 // group ID of owner
	uint32_t st_rdev;	// device type (if inode device)
	uint64_t st_size;	// total size, in bytes
	uint64_t st_blksize; // blocksize for filesystem I/O
	uint64_t st_blocks;  // number of blocks allocated
	time_t st_atime;	 // time of last access
	time_t st_mtime;	 // time of last modification
	time_t st_ctime;	 // time of last change
};

/* GDB uses a specific version of the timeval structure, 12 bytes in size (manual says 8, which is wrong) */
struct __attribute__((packed)) gdb_timeval_t {
	time_t tv_sec;	// second
	uint64_t tv_usec; // microsecond
};

/* GDB Syscall interface */

enum GdbSyscallCommand {
	eGDBSYS_open,
	eGDBSYS_close,
	eGDBSYS_read,
	eGDBSYS_write,
	eGDBSYS_lseek,
	eGDBSYS_rename,
	eGDBSYS_unlink,
	eGDBSYS_stat,
	eGDBSYS_fstat,
	eGDBSYS_gettimeofday,
	eGDBSYS_isatty,
	eGDBSYS_system,
};

/** @brief GDB Syscall completion callback function */
struct GdbSyscallInfo;
typedef void (*gdb_syscall_callback_t)(const GdbSyscallInfo& info);

/** @brief GDB Syscall command information */
struct GdbSyscallInfo {
	GdbSyscallCommand command;		 ///< The syscall command
	gdb_syscall_callback_t callback; ///< User-supplied callback (if any)
	void* param;					 ///< User-supplied parameter for callback
	int result;						 ///< Final result of syscall
	// Command parameters
	union {
		struct {
			const char* filename;
			int flags;
		} open;
		struct {
			int fd;
		} close;
		struct {
			int fd;
			void* buffer;
			size_t bufSize;
		} read;
		struct {
			int fd;
			const void* buffer;
			size_t count;
		} write;
		struct {
			int fd;
			long offset;
			int whence;
		} lseek;
		struct {
			const char* oldpath;
			const char* newpath;
		} rename;
		struct {
			const char* pathname;
		} unlink;
		struct {
			const char* pathname;
			gdb_stat_t* buf;
		} stat;
		struct {
			int fd;
			struct stat* buf;
		} fstat;
		struct {
			gdb_timeval_t* tv;
			void* tz;
		} gettimeofday;
		struct {
			int fd;
		} isatty;
		struct {
			const char* command;
		} system;
	};
};

int gdb_syscall(const GdbSyscallInfo& info);

static inline int gdb_syscall_open(const char* filename, int flags, gdb_syscall_callback_t callback = nullptr,
								   void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_open, callback, param};
	info.open.filename = filename;
	info.open.flags = flags;
	return gdb_syscall(info);
}

static inline int gdb_syscall_close(int fd, gdb_syscall_callback_t callback = nullptr, void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_close, callback, param};
	info.close.fd = fd;
	return gdb_syscall(info);
}

static inline int gdb_syscall_read(int fd, void* buffer, size_t bufSize, gdb_syscall_callback_t callback = nullptr,
								   void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_read, callback, param};
	info.read.fd = fd;
	info.read.buffer = buffer;
	info.read.bufSize = bufSize;
	return gdb_syscall(info);
}

static inline int gdb_syscall_write(int fd, const void* buffer, size_t count, gdb_syscall_callback_t callback = nullptr,
									void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_write, callback, param};
	info.write.fd = fd;
	info.write.buffer = buffer;
	info.write.count = count;
	return gdb_syscall(info);
}

static inline int gdb_syscall_lseek(int fd, long offset, int whence, gdb_syscall_callback_t callback = nullptr,
									void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_lseek, callback, param};
	info.lseek.fd = fd;
	info.lseek.offset = offset;
	info.lseek.whence = whence;
	return gdb_syscall(info);
}

static inline int gdb_syscall_rename(const char* oldpath, const char* newpath,
									 gdb_syscall_callback_t callback = nullptr, void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_rename, callback, param};
	info.rename.oldpath = oldpath;
	info.rename.newpath = newpath;
	return gdb_syscall(info);
}

static inline int gdb_syscall_unlink(const char* pathname, gdb_syscall_callback_t callback = nullptr,
									 void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_unlink, callback, param};
	info.unlink.pathname = pathname;
	return gdb_syscall(info);
}

static inline int gdb_syscall_stat(const char* pathname, gdb_stat_t* buf, gdb_syscall_callback_t callback = nullptr,
								   void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_stat, callback, param};
	info.stat.pathname = pathname;
	info.stat.buf = buf;
	return gdb_syscall(info);
}

static inline int gdb_syscall_fstat(int fd, struct stat* buf, gdb_syscall_callback_t callback = nullptr,
									void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_fstat, callback, param};
	info.fstat.fd = fd;
	info.fstat.buf = buf;
	return gdb_syscall(info);
}

static inline int gdb_syscall_gettimeofday(gdb_timeval_t* tv, void* tz, gdb_syscall_callback_t callback = nullptr,
										   void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_gettimeofday, callback, param};
	info.gettimeofday.tv = tv;
	info.gettimeofday.tz = tz;
	return gdb_syscall(info);
}

static inline int gdb_syscall_isatty(int fd, gdb_syscall_callback_t callback = nullptr, void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_isatty, callback, param};
	info.isatty.fd = fd;
	return gdb_syscall(info);
}

static inline int gdb_syscall_system(const char* command, gdb_syscall_callback_t callback = nullptr,
									 void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_system, callback, param};
	info.system.command = command;
	return gdb_syscall(info);
}

/*
 * IMPORTANT: Reading console will not complete until user types return (or Ctrl+D, Ctrl+C). It must therefore
 * be an asynchronous call or a watchdog reset will occur.
 */
static inline int gdb_console_read(void* buffer, size_t bufSize, gdb_syscall_callback_t callback = nullptr,
								   void* param = nullptr)
{
	return gdb_syscall_read(STDIN_FILENO, buffer, bufSize, callback, param);
}

static inline int gdb_console_write(const void* buffer, size_t count, gdb_syscall_callback_t callback = nullptr,
									void* param = nullptr)
{
	return gdb_syscall_write(STDOUT_FILENO, buffer, count, callback, param);
}

#endif /* _SYSTEM_INCLUDE_GDB_SYSCALL_H_ */
