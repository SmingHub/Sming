/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 15.07.2015
Descr: Implement software SPI. To improve speed, GPIO16 is not supported(see Digital.cpp)
*/
#include "SPISoft.h"
#include <esp_systemapi.h>
#include <FlashString/Array.hpp>

#define PIN_SCK_DEFAULT 14
#define PIN_MISO_DEFAULT 12
#define PIN_MOSI_DEFAULT 13

namespace
{
#ifdef ARCH_ESP8266

#define GP_REG(n) ((n) ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS)
#define GP_IN(pin) ((GPIO_REG_READ(GPIO_IN_ADDRESS) >> (pin)) & 1)
#define GP_OUT(pin, val) GPIO_REG_WRITE(GP_REG(val), BIT(pin))
#define SCK_MOSI_WRITE(val) GPIO_REG_WRITE(GP_REG(val), BIT(pins.sck) | BIT(pins.mosi))

#else

#define GP_IN(pin) digitalRead(pin)
#define GP_OUT(pin, val) digitalWrite(pin, val)
#define SCK_MOSI_WRITE(val)                                                                                            \
	do {                                                                                                               \
		digitalWrite(pins.sck, val);                                                                                   \
		digitalWrite(pins.mosi, val);                                                                                  \
	} while(0)

#endif

#define SCK_SETUP() GP_OUT(pins.sck, !cksample)
#define SCK_SAMPLE() GP_OUT(pins.sck, cksample)
#define SCK_IDLE() GP_OUT(pins.sck, cpol)
#define MOSI_WRITE(d) GP_OUT(pins.mosi, d)
#define MISO_READ() GP_IN(pins.miso)

__forceinline void fastDelay(int d)
{
#ifdef ENABLE_SPISOFT_DELAY
	while(d > 0) {
		__asm__ volatile("nop");
		--d;
	}
#else
	(void)d;
#endif
}

#if defined(ENABLE_SPISOFT_DELAY) && defined(ARCH_ESP8266)

/*
 * Matching small delay values to frequency reuires a lookup table as the relationship is non-linear.
 * Given values are clock frequency in kHz, found by measurement.
 *
 * Two tables are required depending on currently selected CPU frequency.
 */

// clang-format off
DEFINE_FSTR_ARRAY(table80, uint16_t,
	1096,	// 0
	1014,	// 1
	941,	// 2
	880,	// 3
	825,	// 4
	777,	// 5
	734,	// 6
	695,	// 7
	625,	// 8
	568,	// 9
	520,	// 10
)

DEFINE_FSTR_ARRAY(table160, uint16_t,
	1888,	// 0
	1840,	// 1
	1721,	// 2
	1617,	// 3
	1524,	// 4
	1442,	// 5
	1367,	// 6
	1301,	// 7
	1185,	// 8
	1074,	// 9
	994,	// 10
)

// clang-format on

/*
 * Delay values above 10 are calculated, directly proportional to inverse of frequency.
 */

struct CoEfficient {
	int16_t m;
	int16_t c;
};

constexpr int M{16}; // Scalar to keep multipler resolution

constexpr CoEfficient coefficients[2]{
	{int16_t(17 * M), 19},
	{int16_t(8.1 * M), 19},
};

uint8_t checkSpeed(SPISpeed& speed)
{
	constexpr uint32_t minFreq{80000}; // Don't go lower than this

	union SpeedConfig {
		uint32_t val;
		struct {
			uint8_t cpuFreq;
			uint8_t delay;
		};
	};

	SpeedConfig cfg{speed.regVal};

	// A change in CPU frequency or requested frequency will fail this check
	auto cpuFreq = system_get_cpu_freq();
	if(cfg.cpuFreq == cpuFreq) {
		// Already calculated, done
		return cfg.delay;
	}

	uint8_t delay{0};
	uint16_t khz{0};
	auto khzRequired = std::max(speed.frequency, minFreq) / 1000;
	const FSTR::Array<uint16_t>& table = (cpuFreq <= 80) ? table80 : table160;
	if(khzRequired >= table[table.length() - 1]) {
		// Value is in table, match highest frequency not exceeding requested value
		for(uint8_t i = 0; i < table.length(); ++i) {
			khz = table[i];
			if(khz <= khzRequired) {
				delay = i;
				break;
			}
		}
	} else {
		// Calculate the delay, and resulting clock frequency
		auto coeff = coefficients[(cpuFreq <= 80) ? 0 : 1];
		delay = M * ((100000 / khzRequired) - coeff.c) / coeff.m;
#ifdef SPI_DEBUG
		khz = 100000 / ((coeff.m * delay / M) + coeff.c);
#endif
	}

	cfg.cpuFreq = cpuFreq;
	cfg.delay = delay;
	speed.regVal = cfg.val;

#ifdef SPI_DEBUG
	debugf("[SSPI] Using delay %u -> target freq %u -> result %u", delay, speed.frequency, khz * 1000);
#endif

	return cfg.delay;
}

#endif

} // namespace

bool SPISoft::begin()
{
#ifdef ARCH_ESP8266
	if(16 == pins.miso || 16 == pins.mosi || 16 == pins.sck) {
		/*To be able to use fast/simple GPIO read/write GPIO16 is not supported*/
		debugf("SPISoft: GPIO 16 not supported\n");
		return false;
	}
#endif

	if(pins.sck == SPI_PIN_DEFAULT) {
		pins.sck = PIN_SCK_DEFAULT;
	}
	pinMode(pins.sck, OUTPUT);

	if(pins.miso == SPI_PIN_DEFAULT) {
		pins.miso = PIN_MISO_DEFAULT;
	}
	pinMode(pins.miso, INPUT);
	digitalWrite(pins.miso, HIGH);

	if(pins.mosi == SPI_PIN_DEFAULT) {
		pins.mosi = PIN_MOSI_DEFAULT;
	}
	pinMode(pins.mosi, OUTPUT);

	prepare(SPIDefaultSettings);

	return true;
}

uint32_t SPISoft::transferWordLSB(uint32_t word, uint8_t bits)
{
	uint32_t res{0};
	word <<= 32 - bits;
	for(uint32_t mask = BIT(32 - bits); mask != 0; mask <<= 1) {
		uint8_t d = (word & mask) ? 1 : 0;

		// Setup edge, can set both SCK and MOSI together if they're the same value
		if(d != cksample) {
			SCK_MOSI_WRITE(d);
		} else {
			SCK_SETUP();
			MOSI_WRITE(d);
		}
		fastDelay(m_delay);
		res >>= 1;
		SCK_SAMPLE();
		res |= MISO_READ() << 31;
		fastDelay(m_delay - 7);
	}

	res >>= 32 - bits;
	return res;
}

uint32_t SPISoft::transferWordMSB(uint32_t word, uint8_t bits)
{
	uint32_t res{0};

	word <<= 32 - bits;
	for(uint8_t i = 0; i < bits; ++i) {
		uint8_t d = (word >> 31) & 0x01;
		word <<= 1;

		// Setup edge, can set both SCK and MOSI together if they're the same value
		if(d != cksample) {
			SCK_MOSI_WRITE(d);
		} else {
			SCK_SETUP();
			MOSI_WRITE(d);
		}
		fastDelay(m_delay);
		res <<= 1;
		SCK_SAMPLE();
		res |= MISO_READ();
		fastDelay(m_delay - 7);
	}

	return res;
}

uint32_t SPISoft::transfer32(uint32_t val, uint8_t bits)
{
	if(lsbFirst) {
		val = transferWordLSB(val, bits);
	} else {
		val = transferWordMSB(val, bits);
	}

	return val;
}

void SPISoft::transfer(uint8_t* buffer, size_t size)
{
	if(lsbFirst) {
		while(size-- != 0) {
			*buffer = transferWordLSB(*buffer, 8);
			++buffer;
		}
	} else {
		while(size-- != 0) {
			*buffer = transferWordMSB(*buffer, 8);
			++buffer;
		}
	}
}

void SPISoft::prepare(SPISettings& settings)
{
	dataMode = settings.dataMode;
	auto mode_num = SPISettings::getModeNum(dataMode);
	cpol = (mode_num & 0x02) >> 1;
	uint8_t cpha = mode_num & 0x01;
	cksample = (cpol == cpha);

	lsbFirst = (settings.bitOrder != MSBFIRST);
	SCK_SETUP();

#if defined(ENABLE_SPISOFT_DELAY) && defined(ARCH_ESP8266)
	// If user doesn't specify speed then don't override
	if(settings.speed.frequency != 0) {
		m_delay = checkSpeed(settings.speed);
	}
#endif
}

void SPISoft::endTransaction()
{
	SCK_IDLE();
}

bool SPISoft::loopback(bool enable)
{
	(void)enable;
	return false;
}
