#include <user_config.h>
#include <SmingCore.h>
#include "HardwareTimer.h"
#include <gdb_syscall.h>

#define LED_PIN 2 // Note: LED is attached to UART1 TX output

#define TIMERTYPE_HARDWARE 1
#define TIMERTYPE_SIMPLE 2
#define TIMERTYPE_TIMER 3

/*
 * This example uses the hardware timer for best timing accuracy. There is only one of these on the ESP8266,
 * so it may not be available if another module requires it.
 * Most timing applications can use a SimpleTimer, which is good for intervals of up to about 268 seconds.
 * For longer intervals, use a Timer.
 */
#define TIMER_TYPE TIMERTYPE_HARDWARE

/*
 * HardwareTimer defaults to non-maskable mode, so the timer callback cannot be interrupted even by the
 * debugger. To use break/watchpoints we must set the timer to use maskable mode.
 */
#define HWTIMER_TYPE eHWT_Maskable

#if TIMER_TYPE == TIMERTYPE_HARDWARE
HardwareTimer procTimer(HWTIMER_TYPE);
// Hardware timer callbacks must always be in IRAM
#define CALLBACK_ATTR IRAM_ATTR
#elif TIMER_TYPE == TIMERTYPE_SIMPLE
SimpleTimer procTimer;
#define CALLBACK_ATTR GDB_IRAM_ATTR
#else
Timer procTimer;
#define CALLBACK_ATTR GDB_IRAM_ATTR
#endif

bool state = true;

/*
* Notice:  Software breakpoints work only on code that is in RAM.
*          In Sming you have to use the GDB_IRAM_ATTR to do this.
*/
void CALLBACK_ATTR blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
}

void showPrompt()
{
	switch(gdb_present()) {
	case eGDB_Attached:
		Serial.print(_F("\r(Attached) "));
		break;
	case eGDB_Detached:
		Serial.print(_F("\r(Detached) "));
		break;
	case eGDB_NotPresent:
	default:
		Serial.print(_F("\r(Non-GDB) "));
	}
}

bool handleCommand(const String& cmd);

void onDataReceived(Stream& source, char arrivedChar, unsigned short availableCharsCount)
{
	static unsigned commandLength;
	const unsigned MAX_COMMAND_LENGTH = 16;
	static char commandBuffer[MAX_COMMAND_LENGTH + 1];

	// Error detection
	unsigned status = Serial.getStatus();
	if(status != 0) {
		Serial.println();
		if(bitRead(status, eSERS_Overflow)) {
			Serial.println(_F("** RECEIVE OVERFLOW **"));
		}
		if(bitRead(status, eSERS_BreakDetected)) {
			Serial.println(_F("** BREAK DETECTED **"));
		}
		if(bitRead(status, eSERS_FramingError)) {
			Serial.println(_F("** FRAMING ERROR **"));
		}
		if(bitRead(status, eSERS_ParityError)) {
			Serial.println(_F("** PARITY ERROR **"));
		}
		// Discard what is likely to be garbage
		Serial.clear(SERIAL_RX_ONLY);
		commandLength = 0;
		showPrompt();
		return;
	}

	int c;
	while((c = Serial.read()) >= 0) {
		switch(c) {
		case '\b': // delete (backspace)
			if(commandLength > 0) {
				--commandLength;
				Serial.print('\b');
				Serial.print(' ');
				Serial.print('\b');
			}
			break;
		case '\r':
		case '\n':
			//				m_printHex("CMD", commandBuffer, commandLength);
			if(commandLength > 0) {
				Serial.println();
				String cmd(commandBuffer, commandLength);
				commandLength = 0;
				Serial.clear(SERIAL_RX_ONLY);
				handleCommand(cmd);
			}
			showPrompt();
			break;
		default:
			if(c >= 0x20 && c <= 0x7f && commandLength < MAX_COMMAND_LENGTH) {
				commandBuffer[commandLength++] = c;
				Serial.print(char(c));
			}
		}
	}
}

/*
 * Demonstrate opening and reading a file from the host.
 */
void readFile(const char* filename, bool display)
{
	auto start = millis();
	int fd = gdb_syscall_open(filename, O_RDONLY);
	Serial.printf(_F("gdb_syscall_open(\"%s\") = %d\r\n"), filename, fd);
	if(fd > 0) {
		char buf[256];
		size_t total = 0;
		int len;
		do {
			len = gdb_syscall_read(fd, buf, sizeof(buf));
			if(len > 0) {
				total += size_t(len);
				if(display) {
					Serial.write(buf, len);
				}
			}
		} while(len == sizeof(buf));
		auto elapsed = millis() - start;
		Serial.printf(_F("\r\ngdb_syscall_read() = %d, total = %u, elapsed = %u ms, av. %u bytes/sec\r\n"), len, total,
					  elapsed, total == 0 ? 0 : 1000U * total / elapsed);

		gdb_syscall_close(fd);
	}
}

// Forward declaration
void readConsole();

/*
 * Opening, reading and closing a fill are all done using asynchronous syscalls.
 * The initial open() is performed via readFileAsync(), the remaining operations are handled
 * in this callback function.
 */
void asyncReadCallback(const GdbSyscallInfo& info)
{
	// Buffer for performing asynchronous reads
	static char buf[256];

	// State information so we can calculate average throughput
	static struct {
		long start;   // millis() when open() call completed
		size_t total; // Total number of file bytes read so far
	} transfer;

	switch(info.command) {
	case eGDBSYS_open: {
		int fd = info.result;
		String filename(FPSTR(info.open.filename));
		Serial.printf(_F("gdb_syscall_open(\"%s\") = %d\r\n"), filename.c_str(), fd);
		if(fd > 0) {
			transfer.start = millis();
			transfer.total = 0;
			gdb_syscall_read(fd, buf, sizeof(buf), asyncReadCallback);
		}
		break;
	}

	case eGDBSYS_read: {
		//		m_printf(_F("\rgdb_syscall_read() = %d, total = %u"), info.result, prog.total);
		if(info.result > 0) {
			transfer.total += info.result;
		}
		if(info.result == sizeof(buf)) {
			gdb_syscall_read(info.read.fd, buf, sizeof(buf), asyncReadCallback);
		} else {
			gdb_syscall_close(info.read.fd, asyncReadCallback);
		}
		break;
	}

	case eGDBSYS_close: {
		long elapsed = millis() - transfer.start;
		long bps = (transfer.total == 0) ? 0 : 1000U * transfer.total / elapsed;
		Serial.printf(_F("readFileAsync: total = %u, elapsed = %u ms, av. %u bytes/sec\r\n"), transfer.total, elapsed,
					  bps);
		readConsole();
	}

	default:;
	}
}

/*
 * Note that filename must be in persistent memory (e.g. flash string) as call may not be started
 * immediately.
 */
void readFileAsync(const char* filename)
{
	gdb_syscall_open(filename, O_RDONLY, asyncReadCallback);
}

void fileStat(const char* filename)
{
	gdb_stat_t stat;
	int res = gdb_syscall_stat(filename, &stat);
	Serial.printf(_F("gdb_syscall_stat(\"%s\") returned %d\r\n"), filename, res);
	if(res != 0) {
		return;
	}

	Serial.printf(_F("sizeof(stat) == %u, words = %u\r\n"), sizeof(stat), sizeof(stat) / 4);
#define PRT(x) Serial.printf(_F("  " #x " = %u\r\n"), stat.x)
#define PRT_HEX(x) Serial.printf(_F("  " #x " = 0x%08x\r\n"), stat.x)
#define PRT_TIME(x)                                                                                                    \
	Serial.print(_F("  " #x " = "));                                                                                   \
	Serial.println(DateTime(stat.x).toFullDateTimeString());

	PRT(st_dev);
	PRT(st_ino);
	PRT_HEX(st_mode);
	PRT(st_nlink);
	PRT_HEX(st_uid);
	PRT_HEX(st_gid);
	PRT(st_rdev);
	PRT(st_size);
	PRT(st_blksize);
	PRT(st_blocks);
	PRT_TIME(st_atime);
	PRT_TIME(st_mtime);
	PRT_TIME(st_ctime);
#undef PRT
#undef PRT_HEX
#undef PRT_TIME
}

time_t getTimeOfDay()
{
	gdb_timeval_t tv;
	int res = gdb_syscall_gettimeofday(&tv, nullptr);
	if(res < 0) {
		Serial.printf(_F("gdb_syscall_gettimeofday() returned %d\r\n"), res);
		return 0;
	} else {
		Serial.printf(_F("tv_sec = %u, tv_usec = %u, "), tv.tv_sec, uint32_t(tv.tv_usec));
		Serial.println(DateTime(tv.tv_sec).toFullDateTimeString() + _F(" UTC"));
		return tv.tv_sec;
	}
}

/**
 * @brief User typed a command. Deal with it.
 * @retval bool true to continue reading another command
 */
bool handleCommand(const String& cmd)
{
	if(cmd.equalsIgnoreCase(F("readfile1"))) {
		// Read a small file and display it
		readFile(_F("Makefile"), true);
	} else if(cmd.equalsIgnoreCase(F("readfile2"))) {
		// Read a larger file asynchronously and analyse transfer speed
		Serial.println(_F("Please wait..."));
		readFileAsync(PSTR("README.md"));
		return false; // When read has completed, readConsole() will be called again
	} else if(cmd.equalsIgnoreCase(F("stat"))) {
		fileStat(_F("Makefile"));
	} else if(cmd.equalsIgnoreCase(F("time"))) {
		getTimeOfDay();
	} else if(cmd.equalsIgnoreCase(F("ls"))) {
		int res = gdb_syscall_system(PSTR("ls -la"));
		Serial.printf(_F("gdb_syscall_system() returned %d\r\n"), res);
	} else if(cmd.equalsIgnoreCase(F("break"))) {
		gdb_do_break();
	} else if(cmd.equalsIgnoreCase(F("crash"))) {
		Serial.println(_F("Crashing app by writing to address 0\n"
						  "At GDB prompt, enter `set $pc = $pc + 3` to skip offending instruction,\n"
						  "then enter `c` to continue"));
		Serial.flush();
		*(uint8_t*)0 = 0;
		Serial.println("...still running!");
	} else if(cmd.equalsIgnoreCase(F("exit"))) {
		// End console test
		Serial.println(_F("Resuming normal program execution."));
		return false;
	} else if(cmd.equalsIgnoreCase(F("help"))) {
		Serial.print(_F("LiveDebug interactive debugger sample. Available commands:\n"
						"  readfile1 : read and display a test file\n"
						"  readfile2 : read a larger file asynchronously\n"
						"  stat      : issue a 'stat' call\n"
						"  ls        : list directory\n"
						"  break     : break into debugger\n"
						"  crash     : write to address 0x00000000, see what happens\n"
						"  exit      : resume normal application execution\n"));
	} else {
		Serial.println(_F("Unknown command, try 'help'"));
	}

	// Another command, please
	return true;
}

/*
 * Completion callback for console read test. See readConsole().
 *
 * When the syscall is executed, GDB is instructed to read a line of text from the console.
 * GDB implements a line-editor, so information will only be sent when you hit return.
 * Typing Ctrl+D sends the line immediately without any return (note: this doesn't work on Windows.)
 *
 * We continue running until GDB is ready to send the result, which is written to the
 * buffer provided in the original call. This callback function then gets called via the
 * task queue.
 *
 * Data received will include any return character typed.
 */
void onConsoleReadCompleted(const GdbSyscallInfo& info)
{
	Serial.print(_F("gdb_read_console() returned "));
	Serial.print(info.result);
	char* bufptr = static_cast<char*>(info.read.buffer);
	if(info.result <= 0) {
		Serial.println();
		delete bufptr;
	} else {
		unsigned len = info.result;
		if(bufptr[len - 1] == '\n') {
			--len;
		}
		if(len == 0) {
			Serial.println();
		} else {
			String cmd(bufptr, len);
			delete bufptr;
			Serial.print(_F(": \""));
			Serial.print(cmd);
			Serial.println('"');

			if(!handleCommand(cmd)) {
				return;
			}
		}
	}

	// Start another console read
	readConsole();
}

/*
 * Demonstrate GDB console access.
 */
void readConsole()
{
	showPrompt();
	if(gdb_present() == eGDB_Attached) {
		// Issue the syscall
		const unsigned bufsize = 256;
		auto buffer = new char[bufsize];
		int res = gdb_console_read(buffer, bufsize, onConsoleReadCompleted);
		if(res < 0) {
			Serial.printf(_F("gdb_console_read() failed, %d\r\n"), res);
			Serial.println(_F("Is GDBSTUB_ENABLE_SYSCALL enabled ?"));
			Serial.println(_F("Did you build with ENABLE_GDB=1 ?"));
			delete buffer;
			showPrompt();
		}

		/*
		 * GDB executes the system call, finished in onReadCompleted().
		 * Note that any serial output gets ignored by GDB whilst executing a system
		 * call.
		 */
	} else {
		/*
		 * GDB is either detached or not present, serial callback will process input
		 */
	}
}

void GDB_IRAM_ATTR init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.onDataReceived(onDataReceived);

	readConsole();

	pinMode(LED_PIN, OUTPUT);
#if TIMER_TYPE == TIMERTYPE_SIMPLE
	procTimer.setCallback(SimpleTimerCallback(blink));
	procTimer.startMs(1000, true);
#else
	procTimer.initializeMs(1000, blink).start();
#endif
}
