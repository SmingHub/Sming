#include <SmingCore.h>
#include <Libraries/TM1637/TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 14
#define DIO 16

// The amount of time (in milliseconds) between tests
#define TEST_DELAY 500

namespace
{
const uint8_t SEG_DONE[4]{
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,		   // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
	SEG_C | SEG_E | SEG_G,						   // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G		   // E
};
const uint8_t SEG_ALL_OFF[4]{};
const uint8_t SEG_ALL_ON[4]{0xff, 0xff, 0xff, 0xff};

TM1637Display display(CLK, DIO);
SimpleTimer stateTimer;

bool testDisplay()
{
	static uint8_t state;
	static uint8_t subState;

#define STATE(fmt, ...) debug_i("#%u.%u " fmt, state, subState, ##__VA_ARGS__);

	switch(state) {
	case 0: {
		// All segments on
		STATE("All segments on");
		display.setBrightness(7);
		display.setSegments(SEG_ALL_ON);
		break;
	}
	case 1: {
		// Selectively set different digits
		uint8_t pos = 3 - subState;
		STATE("setSegments(data, 1, %u)", pos);
		const uint8_t seg = 0b01001001;
		display.setSegments(&seg, 1, pos);
		if(subState++ < 3) {
			return false;
		}
		break;
	}
	case 2: {
		uint8_t data[]{
			display.encodeDigit(1),
			display.encodeDigit(2),
			display.encodeDigit(3),
		};
		uint8_t length = (subState == 2) ? 3 : 2;
		uint8_t pos = (subState == 0) ? 2 : 1;
		STATE("setSegments([0x%02x, 0x%02x, 0x%02x], %u, %u)", data[0], data[1], data[2], length, pos);
		display.setSegments(data, length, pos);
		if(subState++ < 3) {
			return false;
		}
		break;
	}
	case 3:
	case 4: {
		// Show decimal numbers with/without leading zeros
		static unsigned k;
		if(subState == 0) {
			k = 0;
			++subState;
		}
		bool showLeadingZeroes = (state == 4);
		STATE("showNumberDec(%u, %u)", k, showLeadingZeroes);
		display.showNumberDec(k, showLeadingZeroes);
		k += k * 4 + 7;
		if(k < 10000) {
			return false;
		}
		break;
	}
	case 5: {
		// Run through all the dots
		display.setSegments(SEG_ALL_OFF);
		uint8_t dots = 0x80 >> subState;
		STATE("showNumberDec(0, 0x%02x, true)", dots);
		display.showNumberDecEx(0, dots, true);
		if(subState++ < 4) {
			return false;
		}
		break;
	}
	case 6: {
		// Show decimal number whose length is smaller than 4
		struct NumberArgs {
			uint8_t num;
			bool leadingZero;
			uint8_t length;
			uint8_t pos;
		};
		const NumberArgs arglist[]{
			{153, false, 3, 1}, //
			{22, false, 2, 2},  //
			{0, true, 1, 3},	//
			{0, true, 1, 2},	//
			{0, true, 1, 1},	//
			{0, true, 1, 0},	//
		};
		auto& args = arglist[subState];
		STATE("showNumberDec(%u, %u, %u, %u)", args.num, args.leadingZero, args.length, args.pos);
		display.showNumberDec(args.num, args.leadingZero, args.length, args.pos);
		if(subState++ < ARRAY_SIZE(arglist)) {
			return false;
		}
		break;
	}
	case 7: {
		// Brightness Test
		uint8_t brightness = subState;
		STATE("setBrightness(%u)", brightness);
		display.setBrightness(brightness);
		display.setSegments(SEG_ALL_ON);
		if(++subState < 7) {
			return false;
		}
		break;
	}
	case 8: {
		// On/Off test
		bool onOff = subState & 0x01;
		debug_i("setBrightness(7, %u)", onOff);
		display.setBrightness(7, onOff);
		display.setSegments(SEG_ALL_ON);
		if(++subState < 8) {
			return false;
		}
		break;
	}
	case 9:
		// Done!
		debug_i("Done!");
		display.setSegments(SEG_DONE);
		return true;
	}

	++state;
	subState = 0;
	return false;
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	Serial << _F("Sample TM1637 application") << endl;

#ifdef ARCH_HOST
	setDigitalHooks(nullptr);
#endif

	stateTimer.initializeMs<TEST_DELAY>([]() {
		if(!testDisplay()) {
			stateTimer.startOnce();
		}
	});
	stateTimer.startOnce();
}
