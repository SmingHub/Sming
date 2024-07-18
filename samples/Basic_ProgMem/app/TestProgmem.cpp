/*
 * test_progmem.h
 *
 * @author: 18 Sep 2018 - mikee47 <mike@sillyhouse.net>
 */

#include "TestProgmem.h"
#include <Platform/Timers.h>
#include "FlashData.h"
#include <FlashString/Array.hpp>
#include <FlashString/Vector.hpp>
#include <cinttypes>

#ifndef __INT32
#define __INT32
#endif

// Note: contains nulls which won't display, but will be stored
#define DEMO_TEST_TEXT "This is a flash string -\0Second -\0Third -\0Fourth."

DEFINE_FSTR_LOCAL(demoFSTR1, DEMO_TEST_TEXT);
DEFINE_FSTR_LOCAL(demoFSTR2, DEMO_TEST_TEXT);
DEFINE_PSTR_LOCAL(demoPSTR1, DEMO_TEST_TEXT);

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
	{
		LOAD_PSTR(s, demoPSTR1)
		out << '"' << s << '"' << endl;
	}

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
	out << '"' << buf2 << '"' << endl;

	//
	out.print("> PSTR_ARRAY: ");
	PSTR_ARRAY(psarr, DEMO_TEST_TEXT);
	out.print('"');
	out.write(psarr, sizeof(__pstr__psarr) - 1);
	out.println('"');
	m_printHex("hex", psarr, sizeof(__pstr__psarr));

	//
	out.println("< testPSTR() end\n");
}

void testFSTR(Print& out)
{
	// Demonstrate direct reading of a flashstring
	char buffer[1024];
	auto len = demoFSTR1.readFlash(0, buffer, sizeof(buffer));
	out.print("demoFSTR1.read(): ");
	out.write(buffer, len);
	out.println();

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
	out.print("> demoArray1 : ");
	static constexpr const struct {
		FSTR::ObjectBase object;
		char data[5];
	} demoArray1 PROGMEM = {{5}, {1, 2, 3, 4, 5}};
	auto& arr = demoArray1.object.as<FSTR::Array<uint8_t>>();
	out << arr << endl;

	// Test equality operators
#define TEST(_test) out.printf(_F("%s: %s\n"), (_test) ? _F("PASS") : _F("FAIL"), _F(#_test));
	TEST(demoFSTR1 == demoFSTR2)
	{
		LOAD_PSTR(s, demoPSTR1);
		TEST(demoFSTR1 != s)
	}
	TEST(String(demoFSTR1) == demoFSTR2)
	TEST(demoFSTR1 == String(demoFSTR2))
#undef TEST

	// FSTR table

	DEFINE_FSTR_LOCAL(fstr1, "Test string #1");
	DEFINE_FSTR_LOCAL(fstr2, "Test string #2");

	DEFINE_FSTR_VECTOR_LOCAL(table, FlashString, &fstr1, &fstr2);

	// Table entries may be accessed directly as they are word-aligned
	out.println(_F("FSTR tables -"));
	out << _F(" fstr1 = '") << table[0] << endl;
	out << _F(" fstr1.length() = ") << table[0].length() << endl;
	out << _F(" entries = ") << table.length() << endl;

	out.println("< testFSTR() end\r\n");
}

/*
 * Run a load of iterations for PSTR/FSTR options to illustrate relative performance.
 */
void testSpeed(Print& out)
{
	const unsigned iterations = 200;

	out.printf("Speed tests, %u iterations, times in microseconds\n", iterations);
	ElapseTimer timer;
	uint32_t baseline, elapsed;

	_FPUTS("Baseline test, read string in RAM...");
	unsigned sum = 0;
	timer.start();
	for(unsigned i = 0; i < iterations; ++i) {
		sum += sumBuffer(demoText, sizeof(demoText));
	}
	baseline = timer.elapsedTime();
	out << "Elapsed: " << baseline << ", sum " << sum << endl;

#define END()                                                                                                          \
	elapsed = timer.elapsedTime();                                                                                     \
	out << "Elapsed: " << elapsed << " (baseline + " << elapsed - baseline << "), sum " << sum << endl;

	_FPUTS("Load PSTR into stack buffer...");
	sum = 0;
	timer.start();
	for(unsigned i = 0; i < iterations; ++i) {
		LOAD_PSTR(buf, demoPSTR1);
		sum += sumBuffer(buf, sizeof(buf));
	}
	END()

	_FPUTS("Load PSTR into String...");
	sum = 0;
	timer.start();
	for(unsigned i = 0; i < iterations; ++i) {
		String s(demoFSTR1.data());
		sum += sumBuffer(s.c_str(), s.length() + 1);
	}
	END()

	_FPUTS("Load FlashString into stack buffer...");
	sum = 0;
	timer.start();
	for(unsigned i = 0; i < iterations; ++i) {
		LOAD_FSTR(buf, demoFSTR1);
		sum += sumBuffer(buf, sizeof(buf));
	}
	END()

	_FPUTS("Load FlashString into String...");
	sum = 0;
	timer.start();
	for(unsigned i = 0; i < iterations; ++i) {
		String s(demoFSTR1);
		sum += sumBuffer(s.c_str(), s.length() + 1);
	}
	END()
}

void testProgmem(Print& out)
{
	testPSTR(out);
	testFSTR(out);
	testSpeed(out);
}
