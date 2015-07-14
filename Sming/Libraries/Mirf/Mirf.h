/*
    Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

    $Id$
*/

#ifndef _MIRF_H_
#define _MIRF_H_

#include <Arduino.h>

#include "Mirf_nRF24L01.h"
#include "MirfSpiDriver.h"

// Nrf24l settings

#define mirf_ADDR_LEN	5

class Nrf24l {
	public:
		Nrf24l();

		void init();
		void config();
		void send(uint8_t *value);
		void setRADDR(uint8_t * adr);
		void setTADDR(uint8_t * adr);
		bool dataReady();
		bool isSending();
		bool rxFifoEmpty();
		bool txFifoEmpty();
		void getData(uint8_t * data);
		uint8_t getStatus();
		
		void configRegister(uint8_t reg, uint8_t value);
		void readRegister(uint8_t reg, uint8_t * value, uint8_t len);
		void writeRegister(uint8_t reg, uint8_t * value, uint8_t len);
		void flushTx();
		void powerUpRx();
		void powerUpTx();
		void powerDown();
		
		void nrfSpiWrite(uint8_t reg, uint8_t *data = 0, boolean readData = false, uint8_t len = 0);

		void csnHi();
		void csnLow();

		void ceHi();
		void ceLow();
		void flushRx();

		/**
		 * In sending mode.
		 */
		uint8_t PTX;

		/**
		 * CE Pin controls RX / TX, default 8.
		 */
		uint8_t cePin;

		/**
		 * CSN Pin Chip Select Not, default 7.
		 */
		uint8_t csnPin;

		/**
		 * Channel 0 - 127 or 0 - 84 in the US.
		 */
		uint8_t channel;

		/**
		 * Payload width in bytes default 16 max 32.
		 */
		uint8_t payload;

		/**
		 * The base config register.
		 * When required PWR_UP and/or PRIM_RX will be OR'ed with this.
		 * 
		 * NOTE: Use "_BV(EN_CRC) | _BV(CRCO)" here if you want to
		 *       connect to a device using the RF24 library.
		 */
		uint8_t baseConfig;

		/**
		 * Spi interface (must extend spi).
		 */
		MirfSpiDriver *spi;
};

extern Nrf24l Mirf;

#endif /* _MIRF_H_ */
