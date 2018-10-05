/*
 * test_progmem.h
 *
 * @author: 18 Sep 2018 - mikee47 <mike@sillyhouse.net>
 */

#include "TestProgmem.h"
#include "FlashData.h"
#include "Print.h"
#include "Services/Profiling/ElapseTimer.h"

// Note: contains nulls which won't display, but will be stored
#define DEMO_TEST_TEXT "This is a flash string -\0Second -\0Third -\0Fourth."

static DEFINE_FSTR(demoFSTR1, DEMO_TEST_TEXT);
static DEFINE_FSTR(demoFSTR2, DEMO_TEST_TEXT);
static DEFINE_PSTR(demoPSTR1, DEMO_TEST_TEXT);

static const char demoText[] = DEMO_TEST_TEXT;

// Write a string literal (stored in flash, naturally)
#define _FPUTS(_s) out.write(_F(_s), sizeof(_s) - 1)

/*
 * Sum every character in a buffer - a standard operation for speed testing to ensure optimisation
 * doesn't mess things up
 */
static unsigned sumBuffer(volatile const char* buffer, size_t length)
{
	unsigned sum = 0;
	for(unsigned i = 0; i < length; ++i)
		sum += buffer[i];
	return sum;
}

void testPSTR(Print& out)
{
	//
	out.println(_F("Simple PSTR example using stack\n"));

	//
	out.println(F("Simple PSTR example using heap (String object)\n"));

	// Note that characters after first nul won't be shown ...
	out.print("> demoPSTR1 (print char*): ");
	out.print('"');
	out.print(_FLOAD(demoPSTR1));
	out.println('"');

	// ... now they will: note buf will be aligned up to next dword boundary though
	out.print("> demoPSTR1 (write): ");
	LOAD_PSTR(buf, demoPSTR1);
	out.print('"');
	out.write(buf, sizeof(buf) - 1);
	out.println('"');
	m_printHex("hex", buf, sizeof(buf)); // show the actual data

	// PSR defined in another module - we don't know its size! so have to guess,
	// and of course nuls don't get included
	out.print("> externalPSTR1 (print char*): ");
	char buf2[100];
	strncpy_P(buf2, externalPSTR1, sizeof(buf2));
	buf2[sizeof(buf2) - 1] = '\0';
	out.print('"');
	out.print(buf2);
	out.println('"');

	//
	out.print("> PSTR_ARRAY: ");
	PSTR_ARRAY(psarr, DEMO_TEST_TEXT);
	out.print('"');
	out.write(psarr, sizeof(psarr) - 1);
	out.println('"');
	m_printHex("hex", psarr, sizeof(psarr));

	//
	out.println("< testPSTR() end\n");
}

void testFSTR(Print& out)
{
	// Implicit conversion of FlashString to String; entire content (including nuls) loaded
	out.print("> demoFSTR1 (print String): ");
	out.print('"');
	out.print(demoFSTR1);
	out.println('"');

	// Load the FlashString into a stack buffer
	out.print("> demoFSTR1 (print char*): ");
	LOAD_FSTR(test, demoFSTR1);
	out.print('"');
	out.print(test);
	out.println('"');

	out.print("> demoFSTR1 (write): ");
	out.print('"');
	out.write(test, demoFSTR1.length());
	out.println('"');

	// FlashString defined in another module
	out.print("> externalFSTR1 (print): ");
	out.print('"');
	out.print(externalFSTR1);
	out.println('"');

	// Define a FlashString and load it into a stack buffer in one operation (non-reusable!)
	out.print("> FSTR_ARRAY: ");
	FSTR_ARRAY(fsarr, DEMO_TEST_TEXT);
	out.print('"');
	out.write(fsarr, sizeof(fsarr) - 1);
	out.println('"');

	// Example of array or custom data usage
	out.print("> demoArray1 : {");
	static struct {
		FlashString fstr;
		char data[5];
	} demoArray1 = {{5}, {1, 2, 3, 4, 5}};
	String arr(demoArray1.fstr);
	for(unsigned i = 0; i < arr.length(); ++i) {
		out.print(arr[i], DEC);
		out.print(", ");
	}
	out.println("}");

	// Test equality operators
#define TEST(_test) out.printf(_F("%s: %s\n"), (_test) ? _F("PASS") : _F("FAIL"), _F(#_test));
	TEST(demoFSTR1 == demoFSTR2)
	TEST(demoFSTR1 != _FLOAD(demoPSTR1))
	TEST(String(demoFSTR1) == demoFSTR2)
	TEST(demoFSTR1 == String(demoFSTR2))
#undef TEST


	// FSTR table

	static DEFINE_FSTR(fstr1, "Test string #1");
	static DEFINE_FSTR(fstr2, "Test string #2");

	static FSTR_TABLE(table) = {
			FSTR_PTR(fstr1),
			FSTR_PTR(fstr2),
	};

	// Table entries may be accessed directly as they are word-aligned
	out.println(_F("FSTR tables -"));
	out.printf(_F(" fstr1 = '%s'\n"), String(*table[0]).c_str());
	out.printf(_F(" fstr1.length() = %u\n"), table[0]->length());
	out.printf(_F(" entries = %u\n"), ARRAY_SIZE(table));

	out.println("< testFSTR() end\n");
}

/*
 * Run a load of iterations for PSTR/FSTR options to illustrate relative performance.
 */
void testSpeed(Print& out)
{
	const unsigned iterations = 200;

	out.printf("Speed tests, %u iterations, times in microseconds\n", iterations);
	ElapseTimer timer;
	unsigned tmp = 0;
	uint32_t baseline, elapsed;

	_FPUTS("Baseline test, read string in RAM...");
	timer.start();
	for(unsigned i = 0; i < iterations; ++i)
		tmp += sumBuffer(demoText, sizeof(demoText));
	baseline = timer.elapsed();
	out.printf("Elapsed: %u\n", baseline);

#define END()                                                                                                          \
	elapsed = timer.elapsed();                                                                                         \
	out.printf("Elapsed: %u (baseline + %u)\n", elapsed, elapsed - baseline);

	_FPUTS("Load PSTR into stack buffer...");
	timer.start();
	for(unsigned i = 0; i < iterations; ++i) {
		LOAD_PSTR(buf, demoPSTR1);
		tmp += sumBuffer(buf, sizeof(buf));
	}
	END()

	_FPUTS("Load PSTR into String...");
	timer.start();
	for(unsigned i = 0; i < iterations; ++i) {
		String s(demoFSTR1.data());
		tmp += sumBuffer(s.c_str(), s.length() + 1);
	}
	END()

	_FPUTS("Load FlashString into stack buffer...");
	timer.start();
	for(unsigned i = 0; i < iterations; ++i) {
		LOAD_FSTR(buf, demoFSTR1);
		tmp += sumBuffer(buf, sizeof(buf));
	}
	END()

	_FPUTS("Load FlashString into String...");
	timer.start();
	for(unsigned i = 0; i < iterations; ++i) {
		String s(demoFSTR1);
		tmp += sumBuffer(s.c_str(), s.length() + 1);
	}
	END()
}

void testProgmem(Print& out)
{
	testPSTR(out);
	testFSTR(out);
	testSpeed(out);
}
