/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPI.h
 *
 * Based on Arduino-esp32 code
 *
 * 	https://github.com/espressif/arduino-esp32/blob/master/libraries/SPI/src/SPI.h
 * 	https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-spi.h
 */

/** @defgroup hw_spi SPI Hardware support
 *  @brief    Provides hardware SPI support
 */

#pragma once

#include "SPIBase.h"
#include "SPISettings.h"
#include <soc/soc_caps.h>

//#define SPI_DEBUG  1

// for compatibility when porting from Arduino
#define SPI_HAS_TRANSACTION 1

static constexpr uint8_t SPI_PIN_DEFAULT{0xff};

/**
 * @brief  Hardware SPI object
 * @addtogroup hw_spi
 * @{
 */

/**
 * @brief Identifies bus selection
 */
enum class SpiBus {
	INVALID = 0,
	MIN = 1,
	SPI1 = 1,
	FSPI = 1, // Attached to the flash (can use the same data lines but different SS)
	SPI2 = 2,
	HSPI = 2, // Normally mapped to pins 12 - 15, but can be matrixed to any pins
#if SOC_SPI_PERIPH_NUM > 2
	SPI3 = 3,
	VSPI = 3, // Normally attached to pins 5, 18, 19 and 23, but can be matrixed to any pins
#endif
	MAX = SOC_SPI_PERIPH_NUM,
#ifdef SOC_ESP32C3
	DEFAULT = SPI1,
#elif defined(SOC_ESP32S2)
	DEFAULT = SPI2,
#else
	DEFAULT = VSPI,
#endif
};

/**
 * @brief SPI pin connections
 */
struct SpiPins {
	uint8_t sck{SPI_PIN_DEFAULT};
	uint8_t miso{SPI_PIN_DEFAULT};
	uint8_t mosi{SPI_PIN_DEFAULT};
	uint8_t ss{SPI_PIN_DEFAULT};
};

class SPIClass : public SPIBase
{
public:
	SPIClass(const SPIClass&) = delete;
	SPIClass& operator=(const SPIClass&) = delete;

	SPIClass(SpiBus id = SpiBus::DEFAULT) : busId(id)
	{
	}

	SPIClass(SpiBus id, SpiPins pins) : busId(id), pins(pins)
	{
	}

	/**
	 * @brief Alternative to defining bus and pin set in constructor.
	 * Use this method to change global `SPI` instance setup.
	 *
	 * IMPORTANT: Must be called *before* begin().
	 */
	bool setup(SpiBus id, SpiPins pins);

	bool begin() override;
	void end() override;
	uint8_t read8() override;
	uint32_t transfer32(uint32_t val, uint8_t bits = 32) override;

	using SPIBase::transfer;
	void transfer(uint8_t* buffer, size_t numberBytes) override;

	bool loopback(bool enable);

protected:
	void prepare(SPISettings& settings) override;

private:
	SpiBus busId;
	SpiPins pins;
	bool lsbFirst{false};
};

/** @brief  Global instance of SPI class */
extern SPIClass SPI;
