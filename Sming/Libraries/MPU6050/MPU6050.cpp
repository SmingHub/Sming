// I2Cdev library collection - MPU6050 I2C device class
// Based on InvenSense MPU-6050 register map document rev. 2.0, 5/19/2011
// (RM-MPU-6000A-00) 8/24/2011 by Jeff Rowberg <jeff@rowberg.net> Updates should
// (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     ... - ongoing debug release

// NOTE: THIS IS ONLY A PARIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY
// UNDERGOING ACTIVE DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES.
// PLEASE KEEP THIS IN MIND IF YOU DECIDE TO USE THIS PARTICULAR CODE FOR
// ANYTHING.

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include "MPU6050.h"
#include <string.h>

#define I2C_NUM I2C_NUM_0
using detail::concat;

size_t MPU6050::Motion3::printTo(Print& p) const
{
	size_t n{0};
	n += p.print(x);
	n += p.print('\t');
	n += p.print(y);
	n += p.print('\t');
	n += p.print(z);
	return n;
}

size_t MPU6050::Motion6::printTo(Print& p) const
{
	size_t n{0};
	n += p.print(_F("accel/gyro:\t"));
	n += p.print(accel);
	n += p.print('\t');
	n += p.print(gyro);
	return n;
}

void MPU6050::initialize()
{
	setClockSource(MPU6050_CLOCK_PLL_XGYRO);
	setFullScaleGyroRange(MPU6050_GYRO_FS_250);
	setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
	setSleepEnabled(false); // thanks to Jack Elston for pointing this one out!
}

uint8_t MPU6050::getAccelXSelfTestFactoryTrim()
{
	const uint8_t x = readByte(MPU6050_RA_SELF_TEST_X);
	const uint8_t a = readByte(MPU6050_RA_SELF_TEST_A);
	return (x >> 3) | ((a >> 4) & 0x03);
}

uint8_t MPU6050::getAccelYSelfTestFactoryTrim()
{
	const uint8_t y = readByte(MPU6050_RA_SELF_TEST_Y);
	const uint8_t a = readByte(MPU6050_RA_SELF_TEST_A);
	return (y >> 3) | ((a >> 2) & 0x03);
}

uint8_t MPU6050::getAccelZSelfTestFactoryTrim()
{
	uint8_t buffer[2] = {0};
	I2Cdev::readBytes(devAddr, MPU6050_RA_SELF_TEST_Z, 2, buffer);
	return (buffer[0] >> 3) | (buffer[1] & 0x03);
}

uint8_t MPU6050::getGyroXSelfTestFactoryTrim()
{
	const uint8_t x = readByte(MPU6050_RA_SELF_TEST_X);
	return (x & 0x1F);
}

uint8_t MPU6050::getGyroYSelfTestFactoryTrim()
{
	const uint8_t y = readByte(MPU6050_RA_SELF_TEST_Y);
	return (y & 0x1F);
}

uint8_t MPU6050::getGyroZSelfTestFactoryTrim()
{
	const uint8_t z = readByte(MPU6050_RA_SELF_TEST_Z);
	return (z & 0x1F);
}

// I2C_SLV* registers (Slave 0-3)

/** Get the I2C address of the specified slave (0-3).
 * Note that Bit 7 (MSB) controls read/write mode. If Bit 7 is set, it's a read
 * operation, and if it is cleared, then it's a write operation. The remaining
 * bits (6-0) are the 7-bit device address of the slave device.
 *
 * In read mode, the result of the read is placed in the lowest available
 * EXT_SENS_DATA register. For further information regarding the allocation of
 * read results, please refer to the EXT_SENS_DATA register description
 * (Registers 73 - 96).
 *
 * The MPU-6050 supports a total of five slaves, but Slave 4 has unique
 * characteristics, and so it has its own functions (getSlave4* and setSlave4*).
 *
 * I2C data transactions are performed at the Sample Rate, as defined in
 * Register 25. The user is responsible for ensuring that I2C data transactions
 * to and from each enabled Slave can be completed within a single period of the
 * Sample Rate.
 *
 * The I2C slave access rate can be reduced relative to the Sample Rate. This
 * reduced access rate is determined by I2C_MST_DLY (Register 52). Whether a
 * slave's access rate is reduced relative to the Sample Rate is determined by
 * I2C_MST_DELAY_CTRL (Register 103).
 *
 * The processing order for the slaves is fixed. The sequence followed for
 * processing the slaves is Slave 0, Slave 1, Slave 2, Slave 3 and Slave 4. If a
 * particular Slave is disabled it will be skipped.
 *
 * Each slave can either be accessed at the sample rate or at a reduced sample
 * rate. In a case where some slaves are accessed at the Sample Rate and some
 * slaves are accessed at the reduced rate, the sequence of accessing the slaves
 * (Slave 0 to Slave 4) is still followed. However, the reduced rate slaves will
 * be skipped if their access rate dictates that they should not be accessed
 * during that particular cycle. For further information regarding the reduced
 * access rate, please refer to Register 52. Whether a slave is accessed at the
 * Sample Rate or at the reduced rate is determined by the Delay Enable bits in
 * Register 103.
 *
 * @param num Slave number (0-3)
 * @return Current address for specified slave
 * @see MPU6050_RA_I2C_SLV0_ADDR
 */
uint8_t MPU6050::getSlaveAddress(uint8_t num)
{
	if(num > 3) {
		return 0;
	}
	return readByte(MPU6050_RA_I2C_SLV0_ADDR + num * 3);
}
/** Set the I2C address of the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param address New address for specified slave
 * @see getSlaveAddress()
 * @see MPU6050_RA_I2C_SLV0_ADDR
 */
void MPU6050::setSlaveAddress(uint8_t num, uint8_t address)
{
	if(num > 3) {
		return;
	}
	I2Cdev::writeByte(devAddr, MPU6050_RA_I2C_SLV0_ADDR + num * 3, address);
}
/** Get the active internal register for the specified slave (0-3).
 * Read/write operations for this slave will be done to whatever internal
 * register address is stored in this MPU register.
 *
 * The MPU-6050 supports a total of five slaves, but Slave 4 has unique
 * characteristics, and so it has its own functions.
 *
 * @param num Slave number (0-3)
 * @return Current active register for specified slave
 * @see MPU6050_RA_I2C_SLV0_REG
 */
uint8_t MPU6050::getSlaveRegister(uint8_t num)
{
	if(num > 3) {
		return 0;
	}
	return readByte(MPU6050_RA_I2C_SLV0_REG + num * 3);
}
/** Set the active internal register for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param reg New active register for specified slave
 * @see getSlaveRegister()
 * @see MPU6050_RA_I2C_SLV0_REG
 */
void MPU6050::setSlaveRegister(uint8_t num, uint8_t reg)
{
	if(num > 3) {
		return;
	}
	I2Cdev::writeByte(devAddr, MPU6050_RA_I2C_SLV0_REG + num * 3, reg);
}
/** Get the enabled value for the specified slave (0-3).
 * When set to 1, this bit enables Slave 0 for data transfer operations. When
 * cleared to 0, this bit disables Slave 0 from data transfer operations.
 * @param num Slave number (0-3)
 * @return Current enabled value for specified slave
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
bool MPU6050::getSlaveEnabled(uint8_t num)
{
	if(num > 3) {
		return false;
	}
	return readBit(MPU6050_RA_I2C_SLV0_CTRL + num * 3, MPU6050_I2C_SLV_EN_BIT);
}
/** Set the enabled value for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param enabled New enabled value for specified slave
 * @see getSlaveEnabled()
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
void MPU6050::setSlaveEnabled(uint8_t num, bool enabled)
{
	if(num > 3) {
		return;
	}
	I2Cdev::writeBit(devAddr, MPU6050_RA_I2C_SLV0_CTRL + num * 3, MPU6050_I2C_SLV_EN_BIT, enabled);
}
/** Get word pair byte-swapping enabled for the specified slave (0-3).
 * When set to 1, this bit enables byte swapping. When byte swapping is enabled,
 * the high and low bytes of a word pair are swapped. Please refer to
 * I2C_SLV0_GRP for the pairing convention of the word pairs. When cleared to 0,
 * bytes transferred to and from Slave 0 will be written to EXT_SENS_DATA
 * registers in the order they were transferred.
 *
 * @param num Slave number (0-3)
 * @return Current word pair byte-swapping enabled value for specified slave
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
bool MPU6050::getSlaveWordByteSwap(uint8_t num)
{
	if(num > 3) {
		return false;
	}
	return readBit(MPU6050_RA_I2C_SLV0_CTRL + num * 3, MPU6050_I2C_SLV_BYTE_SW_BIT);
}
/** Set word pair byte-swapping enabled for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param enabled New word pair byte-swapping enabled value for specified slave
 * @see getSlaveWordByteSwap()
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
void MPU6050::setSlaveWordByteSwap(uint8_t num, bool enabled)
{
	if(num > 3) {
		return;
	}
	I2Cdev::writeBit(devAddr, MPU6050_RA_I2C_SLV0_CTRL + num * 3, MPU6050_I2C_SLV_BYTE_SW_BIT, enabled);
}
/** Get write mode for the specified slave (0-3).
 * When set to 1, the transaction will read or write data only. When cleared to
 * 0, the transaction will write a register address prior to reading or writing
 * data. This should equal 0 when specifying the register address within the
 * Slave device to/from which the ensuing data transaction will take place.
 *
 * @param num Slave number (0-3)
 * @return Current write mode for specified slave (0 = register address + data,
 * 1 = data only)
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
bool MPU6050::getSlaveWriteMode(uint8_t num)
{
	if(num > 3) {
		return false;
	}
	return readBit(MPU6050_RA_I2C_SLV0_CTRL + num * 3, MPU6050_I2C_SLV_REG_DIS_BIT);
}
/** Set write mode for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param mode New write mode for specified slave (0 = register address + data,
 * 1 = data only)
 * @see getSlaveWriteMode()
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
void MPU6050::setSlaveWriteMode(uint8_t num, bool mode)
{
	if(num > 3) {
		return;
	}
	I2Cdev::writeBit(devAddr, MPU6050_RA_I2C_SLV0_CTRL + num * 3, MPU6050_I2C_SLV_REG_DIS_BIT, mode);
}
/** Get word pair grouping order offset for the specified slave (0-3).
 * This sets specifies the grouping order of word pairs received from registers.
 * When cleared to 0, bytes from register addresses 0 and 1, 2 and 3, etc (even,
 * then odd register addresses) are paired to form a word. When set to 1, bytes
 * from register addresses are paired 1 and 2, 3 and 4, etc. (odd, then even
 * register addresses) are paired to form a word.
 *
 * @param num Slave number (0-3)
 * @return Current word pair grouping order offset for specified slave
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
bool MPU6050::getSlaveWordGroupOffset(uint8_t num)
{
	if(num > 3) {
		return false;
	}
	return readBit(MPU6050_RA_I2C_SLV0_CTRL + num * 3, MPU6050_I2C_SLV_GRP_BIT);
}
/** Set word pair grouping order offset for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param enabled New word pair grouping order offset for specified slave
 * @see getSlaveWordGroupOffset()
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
void MPU6050::setSlaveWordGroupOffset(uint8_t num, bool enabled)
{
	if(num > 3) {
		return;
	}
	I2Cdev::writeBit(devAddr, MPU6050_RA_I2C_SLV0_CTRL + num * 3, MPU6050_I2C_SLV_GRP_BIT, enabled);
}
/** Get number of bytes to read for the specified slave (0-3).
 * Specifies the number of bytes transferred to and from Slave 0. Clearing this
 * bit to 0 is equivalent to disabling the register by writing 0 to I2C_SLV0_EN.
 * @param num Slave number (0-3)
 * @return Number of bytes to read for specified slave
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
uint8_t MPU6050::getSlaveDataLength(uint8_t num)
{
	if(num > 3) {
		return false;
	}
	return readBits(MPU6050_RA_I2C_SLV0_CTRL + num * 3, MPU6050_I2C_SLV_LEN_BIT, MPU6050_I2C_SLV_LEN_LENGTH);
}
/** Set number of bytes to read for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param length Number of bytes to read for specified slave
 * @see getSlaveDataLength()
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
void MPU6050::setSlaveDataLength(uint8_t num, uint8_t length)
{
	if(num > 3) {
		return;
	}
	I2Cdev::writeBits(devAddr, MPU6050_RA_I2C_SLV0_CTRL + num * 3, MPU6050_I2C_SLV_LEN_BIT, MPU6050_I2C_SLV_LEN_LENGTH,
					  length);
}

MPU6050::Motion6 MPU6050::getMotion6()
{
	Motion6 motion6;
	uint8_t buffer[14] = {0};
	I2Cdev::readBytes(devAddr, MPU6050_RA_ACCEL_XOUT_H, 14, buffer);
	motion6.accel.x = concat(buffer[0], buffer[1]);
	motion6.accel.y = concat(buffer[2], buffer[3]);
	motion6.accel.z = concat(buffer[4], buffer[5]);
	motion6.gyro.x = concat(buffer[8], buffer[9]);
	motion6.gyro.y = concat(buffer[10], buffer[11]);
	motion6.gyro.z = concat(buffer[12], buffer[13]);
	return motion6;
}

MPU6050::Motion3 MPU6050::getAcceleration()
{
	Motion3 accel;
	uint8_t buffer[6] = {0};
	I2Cdev::readBytes(devAddr, MPU6050_RA_ACCEL_XOUT_H, 6, buffer);
	accel.x = concat(buffer[0], buffer[1]);
	accel.y = concat(buffer[2], buffer[3]);
	accel.z = concat(buffer[4], buffer[5]);
	return accel;
}

MPU6050::Motion3 MPU6050::getAngularRate()
{
	Motion3 angularRate;
	uint8_t buffer[6] = {0};
	I2Cdev::readBytes(devAddr, MPU6050_RA_GYRO_XOUT_H, 6, buffer);
	angularRate.x = concat(buffer[0], buffer[1]);
	angularRate.y = concat(buffer[2], buffer[3]);
	angularRate.z = concat(buffer[4], buffer[5]);
	return angularRate;
}

void MPU6050::setSlaveOutputByte(uint8_t num, uint8_t data)
{
	if(num > 3) {
		return;
	}
	I2Cdev::writeByte(devAddr, MPU6050_RA_I2C_SLV0_DO + num, data);
}

bool MPU6050::getSlaveDelayEnabled(uint8_t num)
{
	// MPU6050_DELAYCTRL_I2C_SLV4_DLY_EN_BIT is 4, SLV3 is 3, etc.
	if(num > 4) {
		return false;
	}
	return readBit(MPU6050_RA_I2C_MST_DELAY_CTRL, num);
}

void MPU6050::getFIFOBytes(uint8_t* data, uint8_t length)
{
	if(length > 0) {
		I2Cdev::readBytes(devAddr, MPU6050_RA_FIFO_R_W, length, data);
	} else {
		*data = 0;
	}
}

// XA_OFFS_* registers
int16_t MPU6050::getXAccelOffset()
{
	uint8_t buffer[2] = {0};
	I2Cdev::readBytes(devAddr, MPU6050_RA_XA_OFFS_H, 2, buffer);
	return (((int16_t)buffer[0]) << 8) | buffer[1];
}

// YA_OFFS_* register

int16_t MPU6050::getYAccelOffset()
{
	uint8_t buffer[2] = {0};
	I2Cdev::readBytes(devAddr, MPU6050_RA_YA_OFFS_H, 2, buffer);
	return (((int16_t)buffer[0]) << 8) | buffer[1];
}

// ZA_OFFS_* register

int16_t MPU6050::getZAccelOffset()
{
	uint8_t buffer[2] = {0};
	I2Cdev::readBytes(devAddr, MPU6050_RA_ZA_OFFS_H, 2, buffer);
	return (((int16_t)buffer[0]) << 8) | buffer[1];
}

// BANK_SEL register

void MPU6050::setMemoryBank(uint8_t bank, bool prefetchEnabled, bool userBank)
{
	bank &= 0x1F;
	if(userBank) {
		bank |= 0x20;
	}
	if(prefetchEnabled) {
		bank |= 0x40;
	}
	I2Cdev::writeByte(devAddr, MPU6050_RA_BANK_SEL, bank);
}

// MEM_START_ADDR register

void MPU6050::setMemoryStartAddress(uint8_t address)
{
	I2Cdev::writeByte(devAddr, MPU6050_RA_MEM_START_ADDR, address);
}

// MEM_R_W register

uint8_t MPU6050::readMemoryByte()
{
	return readByte(MPU6050_RA_MEM_R_W);
}

void MPU6050::writeMemoryByte(uint8_t data)
{
	I2Cdev::writeByte(devAddr, MPU6050_RA_MEM_R_W, data);
}

void MPU6050::readMemoryBlock(uint8_t* data, uint16_t dataSize, uint8_t bank, uint8_t address)
{
	setMemoryBank(bank);
	setMemoryStartAddress(address);
	for(uint16_t i = 0; i < dataSize;) {
		// determine correct chunk size according to bank position and data size
		uint8_t chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

		// make sure we don't go past the data size
		if(i + chunkSize > dataSize) {
			chunkSize = dataSize - i;
		}

		// make sure this chunk doesn't go past the bank boundary (256 bytes)
		if(chunkSize > 256 - address) {
			chunkSize = 256 - address;
		}

		// read the chunk of data as specified
		I2Cdev::readBytes(devAddr, MPU6050_RA_MEM_R_W, chunkSize, data + i);

		// increase byte index by [chunkSize]
		i += chunkSize;

		// uint8_t automatically wraps to 0 at 256
		address += chunkSize;

		// if we aren't done, update bank (if necessary) and address
		if(i < dataSize) {
			if(address == 0) {
				bank++;
			}
			setMemoryBank(bank);
			setMemoryStartAddress(address);
		}
	}
}
bool MPU6050::writeMemoryBlock(const uint8_t* data, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify,
							   bool useProgMem)
{
	setMemoryBank(bank);
	setMemoryStartAddress(address);
	uint8_t* verifyBuffer = 0;
	uint8_t* progBuffer = 0;
	uint16_t i;
	uint8_t j;
	if(verify) {
		verifyBuffer = static_cast<uint8_t*>(malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE));
	}
	if(useProgMem) {
		progBuffer = static_cast<uint8_t*>(malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE));
	}
	for(i = 0; i < dataSize;) {
		// determine correct chunk size according to bank position and data size
		uint8_t chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

		// make sure we don't go past the data size
		if(i + chunkSize > dataSize) {
			chunkSize = dataSize - i;
		}

		// make sure this chunk doesn't go past the bank boundary (256 bytes)
		if(chunkSize > 256 - address) {
			chunkSize = 256 - address;
		}

		if(useProgMem) {
			// write the chunk of data as specified
			for(j = 0; j < chunkSize; j++) {
				progBuffer[j] = pgm_read_byte(data + i + j);
			}
		} else {
			// write the chunk of data as specified
			progBuffer = const_cast<uint8_t*>(data) + i;
		}

		I2Cdev::writeBytes(devAddr, MPU6050_RA_MEM_R_W, chunkSize, progBuffer);

		// verify data if needed
		if(verify && verifyBuffer) {
			printf("VERIFY\n");
			setMemoryBank(bank);
			setMemoryStartAddress(address);
			I2Cdev::readBytes(devAddr, MPU6050_RA_MEM_R_W, chunkSize, verifyBuffer);
			if(memcmp(progBuffer, verifyBuffer, chunkSize) != 0) {
				printf("Block write verification error, bank \n");
				/*Serial.print("Block write verification error, bank ");*/
				printf("bank %d", bank);
				printf(", address ");
				printf("%d", address);
				printf("!\nExpected:");
				for(j = 0; j < chunkSize; j++) {
					printf("%#04x", progBuffer[j]);
				}
				printf("\nReceived:");
				for(uint8_t j = 0; j < chunkSize; j++) {
					printf("%#04x", verifyBuffer[i + j]);
				}
				printf("\n");
			}
		}

		// increase byte index by [chunkSize]
		i += chunkSize;

		// uint8_t automatically wraps to 0 at 256
		address += chunkSize;

		// if we aren't done, update bank (if necessary) and address
		if(i < dataSize) {
			if(address == 0) {
				bank++;
			}
			setMemoryBank(bank);
			setMemoryStartAddress(address);
		}
	}
	if(verify) {
		free(verifyBuffer);
	}
	if(useProgMem) {
		free(progBuffer);
	}
	return true;
}
bool MPU6050::writeProgMemoryBlock(const uint8_t* data, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify)
{
	return writeMemoryBlock(data, dataSize, bank, address, verify, false);
}
bool MPU6050::writeDMPConfigurationSet(const uint8_t* data, uint16_t dataSize, bool useProgMem)
{
	uint8_t* progBuffer = 0;
	uint8_t success, special;
	uint16_t i, j;
	if(useProgMem) {
		progBuffer = static_cast<uint8_t*>(malloc(8)); // assume 8-byte blocks, realloc later if necessary
	}

	// config set data is a long string of blocks with the following structure:
	// [bank] [offset] [length] [byte[0], byte[1], ..., byte[length]]
	uint8_t bank, offset, length;
	for(i = 0; i < dataSize;) {
		if(useProgMem) {
			bank = pgm_read_byte(data + i++);
			offset = pgm_read_byte(data + i++);
			length = pgm_read_byte(data + i++);
		} else {
			bank = data[i++];
			offset = data[i++];
			length = data[i++];
		}

		// write data or perform special action
		if(length > 0) {
			// regular block of data to write
			if(useProgMem) {
				if(sizeof(progBuffer) < length)
					progBuffer = static_cast<uint8_t*>(realloc(progBuffer, length));
				for(j = 0; j < length; j++) {
					progBuffer[j] = pgm_read_byte(data + i + j);
				}
			} else {
				progBuffer = const_cast<uint8_t*>(data) + i;
			}
			success = writeMemoryBlock(progBuffer, length, bank, offset, true);
			i += length;
		} else {
			// special instruction
			// NOTE: this kind of behavior (what and when to do certain things)
			// is totally undocumented. This code is in here based on observed
			// behavior only, and exactly why (or even whether) it has to be here
			// is anybody's guess for now.
			if(useProgMem) {
				special = pgm_read_byte(data + i++);
			} else {
				special = data[i++];
			}
			if(special == 0x01) {
				// enable DMP-related interrupts

				// setIntZeroMotionEnabled(true);
				// setIntFIFOBufferOverflowEnabled(true);
				// setIntDMPEnabled(true);
				I2Cdev::writeByte(devAddr, MPU6050_RA_INT_ENABLE,
								  0x32); // single operation

				success = true;
			} else {
				// unknown special command
				success = false;
			}
		}

		if(!success) {
			if(useProgMem) {
				free(progBuffer);
			}
			return false; // uh oh
		}
	}
	if(useProgMem) {
		free(progBuffer);
	}
	return true;
}
/**
 * calibration
 *
 */

/**
  @brief      Fully calibrate Gyro from ZERO in about 6-7 Loops 600-700 readings
*/
void MPU6050::CalibrateGyro(uint8_t Loops)
{
	double kP = 0.3;
	double kI = 90;
	float x;
	x = (100 - map(Loops, 1, 5, 20, 0)) * .01;
	kP *= x;
	kI *= x;

	PID(0x43, kP, kI, Loops);
}

/**
  @brief      Fully calibrate Accel from ZERO in about 6-7 Loops 600-700
  readings
*/
void MPU6050::CalibrateAccel(uint8_t Loops)
{
	float kP = 0.3;
	float kI = 20;
	float x;
	x = (100 - map(Loops, 1, 5, 20, 0)) * .01;
	kP *= x;
	kI *= x;
	PID(0x3B, kP, kI, Loops);
}

/**
 *
 * @param ReadAddress
 * @param kP
 * @param kI
 * @param Loops
 */
void MPU6050::PID(uint8_t ReadAddress, float kP, float kI, uint8_t Loops)
{
	uint8_t SaveAddress = (ReadAddress == 0x3B) ? ((getDeviceID() < 0x38) ? 0x06 : 0x77) : 0x13;

	int16_t Data;
	float Reading;
	int16_t BitZero[3];
	uint8_t shift = (SaveAddress == 0x77) ? 3 : 2;
	float Error, PTerm, ITerm[3];
	uint32_t eSum;
	for(int i = 0; i < 3; i++) {
		I2Cdev::readWord(devAddr, SaveAddress + (i * shift),
						 (uint16_t*)&Data); // reads 1 or more 16 bit integers (Word)
		Reading = Data;
		if(SaveAddress != 0x13) {
			BitZero[i] = Data & 1; // Capture Bit Zero to properly handle Accelerometer calibration
			ITerm[i] = ((float)Reading) * 8;
		} else {
			ITerm[i] = Reading * 4;
		}
	}
	for(int L = 0; L < Loops; L++) {
		int16_t eSample{0};
		for(int c = 0; c < 100; c++) { // 100 PI Calculations
			eSum = 0;
			for(int i = 0; i < 3; i++) {
				I2Cdev::readWord(devAddr, ReadAddress + (i * 2),
								 (uint16_t*)&Data); // reads 1 or more 16 bit integers (Word)
				Reading = Data;
				if((ReadAddress == 0x3B) && (i == 2))
					Reading -= 16384; // remove Gravity
				Error = -Reading;
				eSum += abs(Reading);
				PTerm = kP * Error;
				ITerm[i] += (Error * 0.001) * kI; // Integral term 1000 Calculations a second = 0.001
				if(SaveAddress != 0x13) {
					Data = round((PTerm + ITerm[i]) / 8); // Compute PID Output
					Data = ((Data)&0xFFFE) | BitZero[i];  // Insert Bit0 Saved at beginning
				} else
					Data = round((PTerm + ITerm[i]) / 4); // Compute PID Output
				I2Cdev::writeWord(devAddr, SaveAddress + (i * shift), Data);
			}
			if((c == 99) && eSum > 1000) { // Error is still to great to continue
				c = 0;
			}
			if((eSum * ((ReadAddress == 0x3B) ? .05 : 1)) < 5)
				eSample++; // Successfully found offsets prepare to  advance
			if((eSum < 100) && (c > 10) && (eSample >= 10))
				break; // Advance to next Loop
		}
		kP *= .75;
		kI *= .75;
		for(int i = 0; i < 3; i++) {
			if(SaveAddress != 0x13) {
				Data = round((ITerm[i]) / 8);		 // Compute PID Output
				Data = ((Data)&0xFFFE) | BitZero[i]; // Insert Bit0 Saved at beginning
			} else
				Data = round((ITerm[i]) / 4);
			I2Cdev::writeWord(devAddr, SaveAddress + (i * shift), Data);
		}
	}
	resetFIFO();
	resetDMP();
}

uint8_t MPU6050::readBit(uint8_t regAddr, uint8_t bitNum)
{
	uint8_t bit;
	const auto count = I2Cdev::readBit(devAddr, regAddr, bitNum, &bit);
	(void)count;
	return bit;
}

uint8_t MPU6050::readBits(uint8_t regAddr, uint8_t bitStart, uint8_t length)
{
	uint8_t bits;
	const auto count = I2Cdev::readBits(devAddr, regAddr, bitStart, length, &bits);
	(void)count;
	return bits;
}

uint8_t MPU6050::readByte(uint8_t regAddr)
{
	uint8_t byte;
	const auto count = I2Cdev::readByte(devAddr, regAddr, &byte);
	(void)count;
	return byte;
}
