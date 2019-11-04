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
 * An API is defined for communicating with GDB using the 'file-I/O protocol', but the name is misleading
 * as it can also perform functions not related to file (or console) I/O. The functions are generally
 * POSIX compatible so for further details consult the appropriate reference for your operating system.
 *
 * This is a synchronous protocol, so only one request may be in progress at a time.
 * While a request is in progress GDB will not respond to any notifications from the target - including debug output.
 *
 * All functions are implemented with an optional callback routine, which is essential when reading the
 * console as it can take a considerable time to execute.
 *
 * To use in your application, build with GDBSTUB_ENABLE_SYSCALL=1 and #include this header.
 *
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/File_002dI_002fO-Remote-Protocol-Extension.html
 *
 ****/

#pragma once

#include <user_config.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/time.h>

/** @defgroup gdb_syscall GDB system call API
 *  @ingroup GDB
 *  @{
*/

#pragma pack(push, 4)

// When compiling under Linux, these get #defined and mess things up
#undef st_atime
#undef st_mtime
#undef st_ctime

/**
 * @brief GDB uses a specific version of the stat structure, 64 bytes in size
 */
struct gdb_stat_t {
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

/**
 * @brief GDB uses a specific version of the timeval structure, 12 bytes in size (manual says 8, which is wrong)
 */
struct gdb_timeval_t {
	time_t tv_sec;	// second
	uint64_t tv_usec; // microsecond
};

#pragma pack(pop)

/* GDB Syscall interface */

/**
 * @brief Enumeration defining available commands
 */
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

struct GdbSyscallInfo;

/**
 * @brief GDB Syscall completion callback function
 * @param info Reference to request information, containing result code and user-provided 'param'
 */
typedef void (*gdb_syscall_callback_t)(const GdbSyscallInfo& info);

/**
 * @brief GDB Syscall request information
 */
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
			int mode;
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
			struct gdb_stat_t* buf;
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

/**
 * @brief Stub function to perform a syscall. Implemented by GDB stub.
 * @param info Call request information
 * @retval Error code, < 0 for error or >= 0 for success
 *
 * @note If info.callback is null then the function waits until the request has been completed,
 * and returns the result of the syscall.
 * If info.callback is not null, then this function returns 0 to indicate the request is in progress,
 * or a negative error value to indicate the request could not be started.
 *
 * @note A copy of the request information is made and a reference passed to the completion callback
 * function if one is used. Note that this is _not_ the original structure, so it does not have to be
 * persisent, although any char* or other pointers MUST remain valid until the call is completed.
 */
int gdb_syscall(const GdbSyscallInfo& info);

/**
 * @brief Open a file on the host
 * @param filename Name of file to open, relative to current directory (typically the Sming application folder)
 * @param flags A combination of O_* flags defined in fcntl.h (e.g. O_RDONLY, O_CREAT, etc)
 * @param mode See sys/stat.h
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/open.html
 */
static inline int gdb_syscall_open(const char* filename, int flags, int mode, gdb_syscall_callback_t callback = nullptr,
								   void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_open, callback, param};
	info.open.filename = filename;
	info.open.flags = flags;
	info.open.mode = mode;
	return gdb_syscall(info);
}

/**
 * @brief Close a host file
 * @param fd File handle
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/close.html
 */
static inline int gdb_syscall_close(int fd, gdb_syscall_callback_t callback = nullptr, void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_close, callback, param};
	info.close.fd = fd;
	return gdb_syscall(info);
}

/**
 * @brief Read data from a host file
 * @param fd File handle
 * @param buffer
 * @param bufSize
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/read.html
 */
static inline int gdb_syscall_read(int fd, void* buffer, size_t bufSize, gdb_syscall_callback_t callback = nullptr,
								   void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_read, callback, param};
	info.read.fd = fd;
	info.read.buffer = buffer;
	info.read.bufSize = bufSize;
	return gdb_syscall(info);
}

/**
 * @brief Write data from a host file
 * @param fd File handle
 * @param buffer
 * @param count
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/write.html
 */
static inline int gdb_syscall_write(int fd, const void* buffer, size_t count, gdb_syscall_callback_t callback = nullptr,
									void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_write, callback, param};
	info.write.fd = fd;
	info.write.buffer = buffer;
	info.write.count = count;
	return gdb_syscall(info);
}

/**
 * @brief Get/set current file pointer position in a host file
 * @param fd File handle
 * @param offset Relative to whence
 * @param whence SEEK_SET, SEEK_CUR or SEEK_END
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/lseek.html
 */
static inline int gdb_syscall_lseek(int fd, long offset, int whence, gdb_syscall_callback_t callback = nullptr,
									void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_lseek, callback, param};
	info.lseek.fd = fd;
	info.lseek.offset = offset;
	info.lseek.whence = whence;
	return gdb_syscall(info);
}

/**
 * @brief Rename a host file
 * @param oldpath
 * @param newpath
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/rename.html
 */
static inline int gdb_syscall_rename(const char* oldpath, const char* newpath,
									 gdb_syscall_callback_t callback = nullptr, void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_rename, callback, param};
	info.rename.oldpath = oldpath;
	info.rename.newpath = newpath;
	return gdb_syscall(info);
}

/**
 * @brief Unlink/remove/delete a host file
 * @param pathname
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/unlink.html
 */
static inline int gdb_syscall_unlink(const char* pathname, gdb_syscall_callback_t callback = nullptr,
									 void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_unlink, callback, param};
	info.unlink.pathname = pathname;
	return gdb_syscall(info);
}

/**
 * @brief Obtain information about a host file given its name/path
 * @param pathname
 * @param buf
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/stat_002ffstat.html
 */
static inline int gdb_syscall_stat(const char* pathname, gdb_stat_t* buf, gdb_syscall_callback_t callback = nullptr,
								   void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_stat, callback, param};
	info.stat.pathname = pathname;
	info.stat.buf = buf;
	return gdb_syscall(info);
}

/**
 * @brief Obtain information about a host file given its file handle
 * @param fd
 * @param buf
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/stat_002ffstat.html
 */
static inline int gdb_syscall_fstat(int fd, struct gdb_stat_t* buf, gdb_syscall_callback_t callback = nullptr,
									void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_fstat, callback, param};
	info.fstat.fd = fd;
	info.fstat.buf = buf;
	return gdb_syscall(info);
}

/**
 * @brief Get current time of day from host, in UTC
 * @param tv
 * @param tz Not used, must be null
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/gettimeofday.html
 */
static inline int gdb_syscall_gettimeofday(gdb_timeval_t* tv, void* tz, gdb_syscall_callback_t callback = nullptr,
										   void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_gettimeofday, callback, param};
	info.gettimeofday.tv = tv;
	info.gettimeofday.tz = tz;
	return gdb_syscall(info);
}

/**
 * @brief Determine if the given file handle refers to a console/tty
 * @param fd
 * @param callback
 * @param param
 * @retval int
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/isatty.html
 */
static inline int gdb_syscall_isatty(int fd, gdb_syscall_callback_t callback = nullptr, void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_isatty, callback, param};
	info.isatty.fd = fd;
	return gdb_syscall(info);
}

/**
 * @brief Invoke the 'system' command on the host
 * @param command
 * @param callback
 * @param param
 * @retval int
 * @note For security reasons this command must specifically be enabled using 'set remote system-call-allowed 1'
 * @see https://sourceware.org/gdb/current/onlinedocs/gdb/system.html
 */
static inline int gdb_syscall_system(const char* command, gdb_syscall_callback_t callback = nullptr,
									 void* param = nullptr)
{
	GdbSyscallInfo info = {eGDBSYS_system, callback, param};
	info.system.command = command;
	return gdb_syscall(info);
}

/**
 * @brief Read a line of text from the GDB console
 * @param buffer
 * @param bufSize
 * @param callback
 * @param param
 * @retval int
 * @note IMPORTANT: Reading console will not complete until user types return (or Ctrl+D, Ctrl+C).
 * It must therefore be an asynchronous call or a watchdog reset will occur.
 */
static inline int gdb_console_read(void* buffer, size_t bufSize, gdb_syscall_callback_t callback = nullptr,
								   void* param = nullptr)
{
	return gdb_syscall_read(STDIN_FILENO, buffer, bufSize, callback, param);
}

/**
 * @brief Write text to the GDB console
 * @param buffer
 * @param count
 * @param callback
 * @param param
 * @retval int
 */
static inline int gdb_console_write(const void* buffer, size_t count, gdb_syscall_callback_t callback = nullptr,
									void* param = nullptr)
{
	return gdb_syscall_write(STDOUT_FILENO, buffer, count, callback, param);
}

/** @} */
