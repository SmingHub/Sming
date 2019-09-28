#include <SmingCore.h>
#include "HardwareTimer.h"
#include <gdb/gdb_syscall.h>
#include <Data/Stream/GdbFileStream.h>
#include <Data/Buffer/LineBuffer.h>
#include <Platform/OsMessageInterceptor.h>

#define LED_PIN 2 // Note: LED is attached to UART1 TX output

// Max length of debug command
const unsigned MAX_COMMAND_LENGTH = 64;

#define TIMERTYPE_HARDWARE 1
#define TIMERTYPE_SIMPLE 2
#define TIMERTYPE_TIMER 3

/*
 * This example uses the hardware timer for best timing accuracy. There is only one of these on the ESP8266,
 * so it may not be available if another module requires it.
 *
 * Most software timing applications can use a `SimpleTimer`, which is good for intervals of up to about
 * 429 seconds, or around 2 hours if you compile with USE_US_TIMER=0.
 *
 * For longer intervals and delegate callback support use a `Timer`.
 */
#define TIMER_TYPE TIMERTYPE_HARDWARE

/*
 * We use the timer to blink the LED at this rate
 */
#define BLINK_INTERVAL_MS 1000

/*
 * HardwareTimer defaults to non-maskable mode, so the timer callback cannot be interrupted even by the
 * debugger. To use break/watchpoints we must set the timer to use maskable mode.
 */
#define HWTIMER_TYPE eHWT_Maskable

#if TIMER_TYPE == TIMERTYPE_HARDWARE
HardwareTimer1<TIMER_CLKDIV_16, HWTIMER_TYPE> procTimer;
// Hardware timer callbacks must always be in IRAM
#define CALLBACK_ATTR IRAM_ATTR
#elif TIMER_TYPE == TIMERTYPE_SIMPLE
SimpleTimer procTimer;
#define CALLBACK_ATTR GDB_IRAM_ATTR
#else
Timer procTimer;
#define CALLBACK_ATTR GDB_IRAM_ATTR
#endif

// See blink()
bool ledState = true;

// A simple log file stored on the host
static GdbFileStream logFile;
#define LOG_FILENAME "testlog.txt"

// Handles messages from SDK
static OsMessageInterceptor osMessageInterceptor;

// Supports `consoleOff` command to prevent re-enabling when debugger is attached
bool consoleOffRequested = false;

// Forward declarations
bool handleCommand(const String& cmd);
void readConsole();

/*
* Notice:  Software breakpoints work only on code that is in RAM.
*          In Sming you have to use the GDB_IRAM_ATTR to do this.
*/
void CALLBACK_ATTR blink()
{
	digitalWrite(LED_PIN, ledState);
	ledState = !ledState;
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
		case 0x7f: // xterm ctrl-?
			if(commandLength > 0) {
				--commandLength;
				Serial.print(_F("\b \b"));
			}
			break;
		case '\r':
		case '\n':
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
	int fd = gdb_syscall_open(filename, O_RDONLY, 0);
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

/*
 * A more advanced way to use host File I/O using asynchronous syscalls.
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
 * Read a file using callbacks.
 * Note that filename must be in persistent memory (e.g. flash string) as call may not be started
 * immediately.
 */
void readFileAsync(const char* filename)
{
	gdb_syscall_open(filename, O_RDONLY, 0, asyncReadCallback);
}

void fileStat(const char* filename)
{
	gdb_stat_t stat;
	int res = gdb_syscall_stat(filename, &stat);
	Serial.printf(_F("gdb_syscall_stat(\"%s\") returned %d\r\n"), filename, res);
	if(res != 0) {
		return;
	}

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

/*
 * Keep commands and their description together to ensure 'help' is consistent.
 * This also helps to keep the code clean and easy to read.
 */
#define COMMAND_MAP(XX)                                                                                                \
	XX(readfile1, "Use syscall file I/O functions to read and display a host file\n"                                   \
				  "Calls are blocking so the application is paused during the entire operation")                       \
	XX(readfile2, "Read a larger host file asynchronously\n"                                                           \
				  "Data is processed in a callback function to avoid pausing the application un-necessarily")          \
	XX(stat, "Use `syscall_stat` function to get details for a host file")                                             \
	XX(ls, "Use `syscall_system` function to perform a directory listing on the host")                                 \
	XX(time, "Use `syscall_gettimeofday` to get current time from host")                                               \
	XX(log, "Show state of log file\n"                                                                                 \
			"The log file is written to \"" LOG_FILENAME "\" in the current working directory")                        \
	XX(break, "Demonstrate `gdb_do_break()` function to pause this application and obtain a GDB command prompt\n"      \
			  "Similar to Ctrl+C except we control exactly where the application stops")                               \
	XX(queueBreak, "Demonstrate `gdb_do_break()` function called via task queue\n"                                     \
				   "If you run `bt` you'll see a much smaller stack trace")                                            \
	XX(consoleOff, "Break into debugger and stop reading from console\n"                                               \
				   "Do this if you need to set live breakpoints or observe debug output,\n"                            \
				   "as both are blocked during console reading")                                                       \
	XX(hang, "Enter infinite loop to force a watchdog timeout\n"                                                       \
			 "Tests the crash handler which should display a message,\n"                                               \
			 "then break into the debugger, if available")                                                             \
	XX(read0, "Read from invalid address\n"                                                                            \
			  "Attempting to read from address #0 will trigger a LOAD_PROHIBITED exception")                           \
	XX(write0, "Write to invalid address\n"                                                                            \
			   "Attempting to write to address #0 will trigger a STORE_PROHIBITED exception")                          \
	XX(malloc0, "Call malloc(0)")                                                                                      \
	XX(freetwice, "Free allocated memory twice")                                                                       \
	XX(restart, "Restart the system\n"                                                                                 \
				"GDB should reconnect automatically, but if not run from a terminal.\n"                                \
				"Windows versions of GDB don't handle serial control lines well,\n"                                    \
				"so a nodeMCU, for example, may restart in the wrong mode")                                            \
	XX(disconnect, "Terminates the connection between the debugger and the remote debug target\n"                      \
				   "Calls gdb_detach() - the application will resume normal operation")                                \
	XX(help, "Display this command summary")

/*
 * Macro to simplify command handler function creation.
 * Function returns true to start another 'readConsole' request.
 * If the operation is completed via callback then it returns false instead, and the readConsole called at that point.
 */
#define COMMAND_HANDLER(name) static bool handleCommand_##name()

COMMAND_HANDLER(readfile1)
{
	// Read a small file and display it
	readFile(_F("Makefile"), true);
	return true;
}

COMMAND_HANDLER(readfile2)
{
	// Read a larger file asynchronously and analyse transfer speed
	Serial.println(_F("Please wait..."));
	readFileAsync(PSTR("README.md"));
	return false; // When read has completed, readConsole() will be called again
}

COMMAND_HANDLER(stat)
{
	fileStat(_F("Makefile"));
	return true;
}

COMMAND_HANDLER(time)
{
	gdb_timeval_t tv;
	int res = gdb_syscall_gettimeofday(&tv, nullptr);
	if(res < 0) {
		Serial.printf(_F("gdb_syscall_gettimeofday() returned %d\r\n"), res);
	} else {
		Serial.printf(_F("tv_sec = %u, tv_usec = %u, "), tv.tv_sec, uint32_t(tv.tv_usec));
		Serial.println(DateTime(tv.tv_sec).toFullDateTimeString() + _F(" UTC"));
	}
	return true;
}

COMMAND_HANDLER(log)
{
	if(logFile.isValid()) {
		Serial.printf(_F("Log file is open, size = %u bytes\r\n"), logFile.getPos());
	} else {
		Serial.println(_F("Log file not available"));
	}
	return true;
}

COMMAND_HANDLER(ls)
{
	int res = gdb_syscall_system(PSTR("ls -la"));
	Serial.printf(_F("gdb_syscall_system() returned %d\r\n"), res);
	return true;
}

COMMAND_HANDLER(break)
{
	Serial.println(_F("Calling gdb_do_break()"));
	gdb_do_break();
	return true;
}

COMMAND_HANDLER(queueBreak)
{
	Serial.println(_F("Queuing a call to gdb_do_break()\r\n"
					  "This differs from `break` in that a console read will be in progress when the break is called"));
	System.queueCallback(handleCommand_break);
	return true;
}

COMMAND_HANDLER(consoleOff)
{
	Serial.println(_F("To re-enable console reading, enter `call readConsole()` from GDB prompt"));
	gdb_do_break();
	consoleOffRequested = true;
	return false;
}

COMMAND_HANDLER(hang)
{
	Serial.println(_F("Entering infinite loop..."));
	Serial.flush();
	while(true) {
		//
	}
	return true;
}

COMMAND_HANDLER(read0)
{
	Serial.println(_F("Crashing app by reading from address 0\r\n"
					  "At GDB prompt, enter `set $pc = $pc + 3` to skip offending instruction,\r\n"
					  "then enter `c` to continue"));
	Serial.flush();
	uint8_t value = *(uint8_t*)0;
	Serial.printf("Value at address 0 = 0x%02x\r\n", value);
	return true;
}

COMMAND_HANDLER(write0)
{
	Serial.println(_F("Crashing app by writing to address 0\r\n"
					  "At GDB prompt, enter `set $pc = $pc + 3` to skip offending instruction,\r\n"
					  "then enter `c` to continue"));
	Serial.flush();
	*(uint8_t*)0 = 0;
	Serial.println("...still running!");
	return true;
}

/**
 * @brief See if the OS debug message is something we're interested in.
 * @param msg
 * @retval bool true if we want to report this
 */
static bool __attribute__((noinline)) parseOsMessage(OsMessage& msg)
{
	m_printf(_F("[OS] %s\r\n"), msg.getBuffer());
	if(msg.startsWith(_F("E:M "))) {
		Serial.println(_F("** OS Memory Error **"));
		return true;
	} else if(msg.contains(_F(" assert "))) {
		Serial.println(_F("** OS Assert **"));
		return true;
	} else {
		return false;
	}
}

/**
 * @brief Called when the OS outputs a debug message using os_printf, etc.
 * @param msg The message
 */
static void onOsMessage(OsMessage& msg)
{
	// Note: We do the check in a separate function to avoid messing up the stack pointer
	if(parseOsMessage(msg)) {
		if(gdb_present() == eGDB_Attached) {
			gdb_do_break();
		} else {
#ifdef ARCH_ESP8266
			register uint32_t sp __asm__("a1");
			debug_print_stack(sp + 0x10, 0x3fffffb0);
#endif
		}
	}
}

COMMAND_HANDLER(malloc0)
{
	Serial.println(
		_F("Attempting to allocate a zero-length array results in an OS debug message.\r\n"
		   "The message starts with 'E:M ...' and can often indicate a more serious memory allocation issue."));

	auto mem = os_malloc(0);
	os_free(mem);

	return true;
}

COMMAND_HANDLER(freetwice)
{
	Serial.println(_F("Attempting to free the same memory twice is a common bug.\r\n"
					  "On the test system we see an assertion failure message from the OS."));

	auto mem = static_cast<char*>(os_malloc(123));
	os_free(mem);
	os_free(mem);

	return true;
}

COMMAND_HANDLER(restart)
{
	Serial.println(_F("Restarting...."));
	System.restart();
	return false;
}

COMMAND_HANDLER(disconnect)
{
	// End console test
	Serial.print(_F("Calling gdb_detach() - "));
	if(gdb_present() == eGDB_Attached) {
		Serial.println(_F("resuming normal program execution."));
	} else if(gdb_present() == eGDB_Detached) {
		Serial.println(_F("not attached, so does nothing"));
	} else {
		Serial.println(_F("Application isn't compiled using ENABLE_GDB so this does nothing."));
	}
	Serial.flush();
	gdb_detach();
	return false;
}

COMMAND_HANDLER(help)
{
	Serial.print(_F("LiveDebug interactive debugger sample. Available commands:\r\n"));

	auto print = [](const char* tag, const char* desc) {
		const unsigned indent = 10;
		Serial.print("  ");
		String s(tag);
		s.reserve(indent);
		while(s.length() < indent) {
			s += ' ';
		}
		Serial.print(s);
		Serial.print(" : ");

		// Print multi-line descriptions in sections to maintain correct line indentation
		s.setLength(2 + indent + 3);
		memset(s.begin(), ' ', s.length());

		for(;;) {
			auto end = strchr(desc, '\n');
			if(end == nullptr) {
				Serial.println(desc);
				break;
			} else {
				Serial.write(desc, end - desc);
				Serial.println();
				desc = end + 1;
				Serial.print(s);
			}
		}
	};

#define XX(tag, desc) print(_F(#tag), _F(desc));
	COMMAND_MAP(XX)
#undef XX
	return true;
}

/**
 * @brief User typed a command. Deal with it.
 * @retval bool true to continue reading another command
 */
bool handleCommand(const String& cmd)
{
	if(logFile.isValid()) {
		logFile.print(_F("handleCommand('"));
		logFile.print(cmd);
		logFile.println(_F("')"));
	}

#define XX(tag, desc)                                                                                                  \
	if(cmd.equalsIgnoreCase(F(#tag))) {                                                                                \
		return handleCommand_##tag();                                                                                  \
	}
	COMMAND_MAP(XX)
#undef XX

	Serial.printf(_F("Unknown command '%s', try 'help'\r\n"), cmd.c_str());
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
	int result = info.result;
	char* bufptr = static_cast<char*>(info.read.buffer);

	debug_i("gdb_read_console() returned %d", result);
	if(result > 0) {
		// Remove trailing newline character
		unsigned len = result;
		if(bufptr[len - 1] == '\n') {
			--len;
		}

		if(len > 0) {
			String cmd(bufptr, len);
			if(!handleCommand(cmd)) {
				return; // Don't call readConsole
			}
		}
	}

	// Start another console read
	readConsole();
}

/*
 * Demonstrate GDB console access.
 * We actually queue this so it can be called directly from GDB to re-enable console reading
 * after using the `consoleOff` command.
 */
void readConsole()
{
	consoleOffRequested = false;
	System.queueCallback(InterruptCallback([]() {
		showPrompt();
		if(gdb_present() == eGDB_Attached) {
			// Issue the syscall
			static char buffer[MAX_COMMAND_LENGTH];
			int res = gdb_console_read(buffer, MAX_COMMAND_LENGTH, onConsoleReadCompleted);
			if(res < 0) {
				Serial.printf(_F("gdb_console_read() failed, %d\r\n"), res);
				Serial.println(_F("Is GDBSTUB_ENABLE_SYSCALL enabled ?"));
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
	}));
}

extern "C" void gdb_on_attach(bool attached)
{
	debug_i("GdbAttach(%d)", attached);
	if(attached) {
		// Open a log file on the host to demonstrate use of GdbFileStream
		logFile.open(F(LOG_FILENAME), eFO_WriteOnly | eFO_CreateIfNotExist);
		debug_i("open log %d", logFile.getLastError());
		logFile.println();

		logFile.println(_F("\r\n=== OPENED ==="));
		gdb_timeval_t tv;
		gdb_syscall_gettimeofday(&tv, nullptr);
		logFile.println(DateTime(tv.tv_sec).toFullDateTimeString());

		// Start interacting with GDB
		if(!consoleOffRequested) {
			readConsole();
		}
	} else {
		// Note: GDB is already detached so underlying call to gdb_syscall_close() will fail silently
		logFile.close();
	}
}

static void printTimerDetails()
{
	Serial.print(procTimer);
	Serial.print(", maxTicks = ");
	Serial.print(procTimer.maxTicks());
	Serial.print(", maxTime = ");
	Serial.print(procTimer.micros().ticksToTime(procTimer.maxTicks()).value());
	Serial.println();
}

void GDB_IRAM_ATTR init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.onDataReceived(onDataReceived);
	Serial.systemDebugOutput(true);

	Serial.println(_F("LiveDebug sample\r\n"
					  "Explore some capabilities of the GDB debugger.\r\n"));

	// Install a debug output hook to monitor OS debug messages
	osMessageInterceptor.begin(onOsMessage);

	if(gdb_present() != eGDB_Attached) {
		System.onReady(showPrompt);
	}

	pinMode(LED_PIN, OUTPUT);
	procTimer.initializeMs<BLINK_INTERVAL_MS>(blink).start();

	printTimerDetails();
}
