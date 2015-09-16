/*
 * SI4432 library for Arduino - v0.1
 *
 * Please note that Library uses standart SS pin for NSEL pin on the chip. This is 53 for Mega, 10 for Uno.
 * NOTES:
 *
 * V0.1
 * * Library supports no `custom' changes and usages of GPIO pin. Modify/add/remove your changes if necessary
 * * Radio use variable packet field format with 4 byte address header, first data field as length. Change if necessary
 *
 * made by Ahmet (theGanymedes) Ipkin
 *
 * 2014
 */

/*
Modified by: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 15.08.2015
Descr: Header for radio module Si4432 aka RF22 driver
Link: http://www.electrodragon.com/w/SI4432_433M-Wireless_Transceiver_Module_%281.5KM_Range,_Shield-Protected%29

Original author license: MIT (https://github.com/theGanymedes/si4432/issues/2)
Original location: https://github.com/theGanymedes/si4432/
*/
#include "si4432.h"

#define MAX_TRANSMIT_TIMEOUT 200

#define BAUD_RATE_REGS 12
struct baudRateCfg
{
	uint8_t reg[BAUD_RATE_REGS];
};

baudRateCfg BaudRates[e_Baud_numBauds] =
{
	//0x1C  0x20  0x21  0x22   0x23  0x24  0x25  0x6E  0x6F  0x70  0x71  0x72
	{ 0x02, 0x68, 0x01, 0x3A, 0x93, 0x04, 0xEE, 0x09, 0xD5, 0x0C, 0x23, 0x1F}, //38k4
	{ 0x82, 0x68, 0x01, 0x3A, 0x93, 0x04, 0xEE, 0x1D, 0x7E, 0x0C, 0x23, 0x5C}, //115k2
	{ 0x8B, 0x34, 0x02, 0x75, 0x25, 0x07, 0xFF, 0x3A, 0xFB, 0x0C, 0x23, 0xB8} //230k4
};

//values here are kept in khz x 10 format (for not to deal with decimals) - look at AN440 page 26 for whole table
const uint16_t IFFilterTable[][2] = { { 322, 0x26 }, { 3355, 0x88 }, { 3618, 0x89 }, { 4202, 0x8A }, { 4684, 0x8B }, {
		5188, 0x8C }, { 5770, 0x8D }, { 6207, 0x8E } };

Si4432::Si4432(SPISoft *pSpi, uint8_t InterruptPin) :
		_spi(pSpi), _sdnPin(0), _intPin(InterruptPin), _freqCarrier(433000000), _freqChannel(0), _kbps(eBaud_38k4), _packageSign(
				0xDEAD) { // default is 450 mhz
}

void Si4432::setFrequency(unsigned long baseFrequencyMhz) {

	if ((baseFrequencyMhz < 240) || (baseFrequencyMhz > 930))
		return; // invalid frequency

	_freqCarrier = baseFrequencyMhz;
	byte highBand = 0;
	if (baseFrequencyMhz >= 480) {
		highBand = 1;
	}

	double fPart = (baseFrequencyMhz / (10 * (highBand + 1))) - 24;

	uint8_t freqband = (uint8_t) fPart; // truncate the int

	uint16_t freqcarrier = (fPart - freqband) * 64000;

	// sideband is always on (0x40) :
	byte vals[3] = { (byte)(0x40 | (highBand << 5) | (freqband & 0x3F)),
					(byte)(freqcarrier >> 8),
					(byte)(freqcarrier & 0xFF) };

	BurstWrite(REG_FREQBAND, vals, 3);

}

void Si4432::setCommsSignature(uint16_t signature) {
	_packageSign = signature;

	ChangeRegister(REG_TRANSMIT_HEADER3, _packageSign >> 8); // header (signature) byte 3 val
	ChangeRegister(REG_TRANSMIT_HEADER2, (_packageSign & 0xFF)); // header (signature) byte 2 val

	ChangeRegister(REG_CHECK_HEADER3, _packageSign >> 8); // header (signature) byte 3 val for receive checks
	ChangeRegister(REG_CHECK_HEADER2, (_packageSign & 0xFF)); // header (signature) byte 2 val for receive checks

#if DEBUG_SI4432
	debugf("Package signature is set!");
#endif
}

void Si4432::init() {

	if (_intPin != 0)
		pinMode(_intPin, INPUT);

	_spi->begin();
	_spi->setDelay(200);

#if DEBUG_SI4432
	debugf("SPI is initialized now.");
#endif

	hardReset();

}

void Si4432::boot() {
	/*
	 byte currentFix[] = { 0x80, 0x40, 0x7F };
	 BurstWrite(REG_CHARGEPUMP_OVERRIDE, currentFix, 3); // refer to AN440 for reasons

	 ChangeRegister(REG_GPIO0_CONF, 0x0F); // tx/rx data clk pin
	 ChangeRegister(REG_GPIO1_CONF, 0x00); // POR inverted pin
	 ChangeRegister(REG_GPIO2_CONF, 0x1C); // clear channel pin
	 */
	ChangeRegister(REG_AFC_TIMING_CONTROL, 0x02); // refer to AN440 for reasons
	ChangeRegister(REG_AFC_LIMITER, 0xFF); // write max value - excel file did that.
	ChangeRegister(REG_AGC_OVERRIDE, 0x60); // max gain control
	ChangeRegister(REG_AFC_LOOP_GEARSHIFT_OVERRIDE, 0x3C); // turn off AFC
	ChangeRegister(REG_DATAACCESS_CONTROL, 0xAD); // enable rx packet handling, enable tx packet handling, enable CRC, use CRC-IBM
	ChangeRegister(REG_HEADER_CONTROL1, 0x0C); // no broadcast address control, enable check headers for bytes 3 & 2
	ChangeRegister(REG_HEADER_CONTROL2, 0x22);  // enable headers byte 3 & 2, no fixed package length, sync word 3 & 2
	ChangeRegister(REG_PREAMBLE_LENGTH, 0x08); // 8 * 4 bits = 32 bits (4 bytes) preamble length
	ChangeRegister(REG_PREAMBLE_DETECTION, 0x3A); // validate 7 * 4 bits of preamble  in a package
	ChangeRegister(REG_SYNC_WORD3, 0x2D); // sync byte 3 val
	ChangeRegister(REG_SYNC_WORD2, 0xD4); // sync byte 2 val

	ChangeRegister(REG_TX_POWER, 0x1F); // max power

	ChangeRegister(REG_CHANNEL_STEPSIZE, 0x64); // each channel is of 1 Mhz interval

	//setFrequency(_freqCarrier); // default freq

	//set frequency to 433Mhz
	ChangeRegister(REG_FREQBAND, 0x53);
	ChangeRegister(REG_FREQCARRIER_H, 0x0);
	ChangeRegister(REG_FREQCARRIER_L, 0x0);

	setBaudRateFast(_kbps); // default baud rate is 38k4
	setChannel(_freqChannel); // default channel is 0
	setCommsSignature(_packageSign); // default signature

	switchMode(Ready);
}

bool Si4432::sendPacket(uint8_t length,
						const byte* data,
						bool waitResponse/* = false*/,
						uint32_t ackTimeout/* = 100*/,
						uint8_t *responseLength/* = 0*/,
						byte* responseBuffer/* = 0*/)
{
	clearTxFIFO();
	ChangeRegister(REG_PKG_LEN, length);

	BurstWrite(REG_FIFO, data, length);

	ChangeRegister(REG_INT_ENABLE1, 0x04); // set interrupts on for package sent
	ChangeRegister(REG_INT_ENABLE2, 0x00); // set interrupts off for anything else
	//read interrupt registers to clean them
	ReadRegister(REG_INT_STATUS1);
	ReadRegister(REG_INT_STATUS2);

	switchMode(TXMode | Ready);

	uint64_t enterMillis = millis();

	while (millis() - enterMillis < MAX_TRANSMIT_TIMEOUT) {

		if ((_intPin != 0) && (digitalRead(_intPin) != 0)) {
			continue;
		}

		byte intStatus = ReadRegister(REG_INT_STATUS1);
		ReadRegister(REG_INT_STATUS2);

		if (intStatus & 0x04) {
			switchMode(Ready | TuneMode);
#if DEBUG_SI4432
			debugf("Package sent! -- %x ", intStatus);
#endif
			// package sent. now, return true if not to wait ack, or wait ack (wait for packet only for 'remaining' amount of time)
			if (waitResponse) {
				if (waitForPacket(ackTimeout)) {
					getPacketReceived(responseLength, responseBuffer);
					return true;
				} else {
					return false;
				}
			} else {
				return true;
			}
		}
	}

	//timeout occured.
	debugf("TX timeout");

	switchMode(Ready);

	if (ReadRegister(REG_DEV_STATUS) & 0x80) {
		clearFIFO();
	}

	return false;

}

bool Si4432::waitForPacket(uint64_t waitMs) {

	startListening();

	uint64_t enterMillis = millis();
	while (millis() - enterMillis < waitMs) {

		if (!isPacketReceived()) {
			continue;
		} else {
			return true;
		}

	}
	//timeout occured.

	debugf("RX timeout");

	switchMode(Ready);
	clearRxFIFO();

	return false;
}

void Si4432::getPacketReceived(uint8_t* length, byte* readData) {

	*length = ReadRegister(REG_RECEIVED_LENGTH);

	BurstRead(REG_FIFO, readData, *length);

	clearRxFIFO(); // which will also clear the interrupts
}

void Si4432::setChannel(byte channel) {

	ChangeRegister(REG_FREQCHANNEL, channel);

}

void Si4432::switchMode(byte mode) {

	ChangeRegister(REG_STATE, mode); // receive mode
	//delay(20);
#if DEBUG_SI4432
	delay(1);
	byte val = ReadRegister(REG_DEV_STATUS);
	debugf("== DEV STATUS: %x ==", val);
#endif
}

void Si4432::ChangeRegister(Registers reg, byte value) {
	BurstWrite(reg, &value, 1);
}

void Si4432::setBaudRateFast(eBaudRate baud)
{
	_kbps = baud;

	BurstWrite(REG_IF_FILTER_BW, &(BaudRates[baud].reg[0]), 1);

	BurstWrite(REG_CLOCK_RECOVERY_OVERSAMPLING, &(BaudRates[baud].reg[1]), 6);

	BurstWrite(REG_TX_DATARATE1, &(BaudRates[baud].reg[7]), 5);
}

void Si4432::setBaudRate(uint16_t kbps) {

	// chip normally supports very low bps values, but they are cumbersome to implement - so I just didn't implement lower bps values
	if ((kbps > 256) || (kbps < 1))
		return;

	byte freqDev = kbps <= 10 ? 15 : 150;		// 15khz / 150 khz
	byte modulationValue = kbps < 30 ? 0x4c : 0x0c;		// use FIFO Mode, GFSK, low baud mode on / off

	byte modulationVals[] = { modulationValue, 0x23, (byte)(0.5f + (freqDev * 1000.0) / 625.0) }; // msb of the kpbs to 3rd bit of register
	BurstWrite(REG_MODULATION_MODE1, modulationVals, 3);

	// set data rate
	uint16_t bpsRegVal = (uint16_t)(0.5 + ((kbps * (kbps < 30 ? 2097152 : 65536.0)) / 1000.0));
	byte datarateVals[] = { (byte)(bpsRegVal >> 8), (byte)(bpsRegVal & 0xFF) };

	BurstWrite(REG_TX_DATARATE1, datarateVals, 2);

	//now set the timings
	uint16_t minBandwidth = (2 * (uint32_t) freqDev) + kbps;
#if DEBUG_SI4432
	debugf("min Bandwidth value: %x ", minBandwidth);
#endif
	byte IFValue = 0xff;
	//since the table is ordered (from low to high), just find the 'minimum bandwith which is greater than required'
	for (byte i = 0; i < 8; ++i) {
		if (IFFilterTable[i][0] >= (minBandwidth * 10)) {
			IFValue = IFFilterTable[i][1];
			break;
		}
	}
#if DEBUG_SI4432
	debugf("Selected IF value: %x ", IFValue);
#endif

	ChangeRegister(REG_IF_FILTER_BW, IFValue);

	byte dwn3_bypass = (IFValue & 0x80) ? 1 : 0; // if msb is set
	byte ndec_exp = (IFValue >> 4) & 0x07; // only 3 bits

	uint16_t rxOversampling = 0.5 + ((500.0 * (1 + 2 * dwn3_bypass)) / ((pow(2, ndec_exp - 3)) * (double ) kbps));

	uint32_t ncOffset = ceil(((double) kbps * (pow(2, ndec_exp + 20))) / (500.0 * (1 + 2 * dwn3_bypass)));

	uint16_t crGain = 2 + ((65535 * (int64_t) kbps) / ((int64_t) rxOversampling * freqDev));
	byte crMultiplier = 0x00;
	if (crGain > 0x7FF) {
		crGain = 0x7FF;
	}
#if DEBUG_SI4432
	debugf("dwn3_bypass value: %x ", dwn3_bypass);
	debugf("ndec_exp value: %x ", ndec_exp);
	debugf("rxOversampling value: %x ", rxOversampling);
	debugf("ncOffset value: %x ", ncOffset);
	debugf("crGain value: %x ", crGain);
	debugf("crMultiplier value: %x ", crMultiplier);

#endif

	byte timingVals[] = { (byte)(rxOversampling & 0xFF),
							(byte)(((rxOversampling & 0x0700) >> 3) | ((ncOffset >> 16) & 0x0F)),
			(byte)((ncOffset >> 8) & 0xFF), (byte)(ncOffset & 0xFF), (byte)(((crGain & 0x0700) >> 8) | crMultiplier), (byte)(crGain & 0xFF) };

	BurstWrite(REG_CLOCK_RECOVERY_OVERSAMPLING, timingVals, 6);

}

byte Si4432::ReadRegister(Registers reg) {
	byte val = 0xFF;
	BurstRead(reg, &val, 1);
	return val;
}

void Si4432::BurstWrite(Registers startReg, const byte value[], uint8_t length) {

	byte regVal = (byte) startReg | 0x80; // set MSB

	_spi->enable();
	delayMicroseconds(1);
	_spi->send(&regVal, 1);

#if DEBUG_VERBOSE_SI4432
		debugf("Writing: %x | %x ... %x (%d bytes)", (regVal != 0xFF ? (regVal) & 0x7F : 0x7F),
				value[0], value[length-1], length);
#endif

	_spi->send(value, length);

	_spi->disable();
}

void Si4432::BurstRead(Registers startReg, byte value[], uint8_t length) {

	byte regVal = (byte) startReg & 0x7F; // set MSB

	_spi->enable();
	delayMicroseconds(1);
	_spi->send(&regVal, 1);

	_spi->setMOSI(HIGH); /* Send 0xFF */
	_spi->recv(value, length);

#if DEBUG_VERBOSE_SI4432
		debugf("Reading: %x  | %x..%x (%d bytes)", (regVal != 0x7F ? (regVal) & 0x7F : 0x7F),
				value[0], value[length-1], length);
#endif
	_spi->disable();
}

void Si4432::readAll() {

	byte allValues[0x80];

	BurstRead(REG_DEV_TYPE, allValues, 0x7F);

	debugf("REGS  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");

	for (byte i = 0; i < 0x7f; i+=16)
	{
		debugf("(%02x): %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", i,
				(int ) allValues[i+0], (int ) allValues[i+1], (int ) allValues[i+2], (int ) allValues[i+3],
				(int ) allValues[i+4], (int ) allValues[i+5], (int ) allValues[i+6], (int ) allValues[i+7],
				(int ) allValues[i+8], (int ) allValues[i+9], (int ) allValues[i+10], (int ) allValues[i+11],
				(int ) allValues[i+12], (int ) allValues[i+13], (int ) allValues[i+14], (int ) allValues[i+15]
				);
	}
}

void Si4432::clearTxFIFO() {
	ChangeRegister(REG_OPERATION_CONTROL, 0x01);
	ChangeRegister(REG_OPERATION_CONTROL, 0x00);

}

void Si4432::clearRxFIFO() {
	ChangeRegister(REG_OPERATION_CONTROL, 0x02);
	ChangeRegister(REG_OPERATION_CONTROL, 0x00);
}

void Si4432::clearFIFO() {
	ChangeRegister(REG_OPERATION_CONTROL, 0x03);
	ChangeRegister(REG_OPERATION_CONTROL, 0x00);
}

void Si4432::softReset() {

	switchMode(0x80);//chip reset
	delay(1);

	byte reg = ReadRegister(REG_INT_STATUS2);
	while ((reg & 0x02) != 0x02) {
		delay(1);
		reg = ReadRegister(REG_INT_STATUS2);
	}

	boot();

}

void Si4432::hardReset() {


	switchMode(0x80);//chip reset
	delay(1);

	byte reg = ReadRegister(REG_INT_STATUS2);
	uint8_t count = 25;
	while ((reg & 0x02) != 0x02 && reg != 0xFF && --count)
	{
#if DEBUG_SI4432
		debugf("POR: %x ", reg);
		delay(1);
		reg = ReadRegister(REG_INT_STATUS2);
#endif
	}

	boot();
}

void Si4432::startListening() {

	clearRxFIFO(); // clear first, so it doesn't overflow if packet is big

	ChangeRegister(REG_INT_ENABLE1, 0x03); // set interrupts on for package received and CRC error

#if DEBUG_SI4432
	ChangeRegister(REG_INT_ENABLE2, 0xC0);
#else
	ChangeRegister(REG_INT_ENABLE2, 0x00); // set other interrupts off
#endif
	//read interrupt registers to clean them
	ReadRegister(REG_INT_STATUS1);
	ReadRegister(REG_INT_STATUS2);

	switchMode(RXMode | Ready);
}

bool Si4432::isPacketReceived() {

	if ((_intPin != 0) && (digitalRead(_intPin) != 0)) {
		return false; // if no interrupt occured, no packet received is assumed (since startListening will be called prior, this assumption is enough)
	}
	// check for package received status interrupt register
	byte intStat = ReadRegister(REG_INT_STATUS1);

#if DEBUG_SI4432
	byte intStat2 = ReadRegister(REG_INT_STATUS2);

	if (intStat2 & 0x40) { //interrupt occured, check it && read the Interrupt Status1 register for 'preamble '

		debugf("Valid Preamble detected -- %x", intStat2);

	}

	if (intStat2 & 0x80) { //interrupt occured, check it && read the Interrupt Status1 register for 'preamble '

		debugf("SYNC WORD detected -- %x", intStat2);

	}
#else
	ReadRegister(REG_INT_STATUS2);
#endif

	if (intStat & 0x02) { //interrupt occured, check it && read the Interrupt Status1 register for 'valid packet'
		switchMode(Ready | TuneMode); // if packet came, get out of Rx mode till the packet is read out. Keep PLL on for fast reaction
#if DEBUG_SI4432
				debugf("Packet detected -- %x", intStat);
#endif
		return true;
	} else if (intStat & 0x01) { // packet crc error
		switchMode(Ready); // get out of Rx mode till buffers are cleared
//#if DEBUG_SI4432
		debugf("CRC Error in Packet detected!-- %x ", intStat);
//#endif
		clearRxFIFO();
		switchMode(RXMode | Ready); // get back to work
		return false;
	}

	//no relevant interrupt? no packet!

	return false;
}

