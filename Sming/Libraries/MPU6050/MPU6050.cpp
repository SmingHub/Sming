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
#include <cassert>

#define I2C_NUM I2C_NUM_0
using detail::concat;

namespace
{
//Slave 4’s characteristics differ greatly from those of Slaves 0-3.
//Hence our API support only up to slave 3
constexpr uint8_t MAX_SLAVE_ID{3};
#define ASSERT_SLAVE_ID_VALID(slaveId) assert((slaveId <= MAX_SLAVE_ID))
} // namespace

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

uint8_t MPU6050::getSlaveAddress(SlaveId slaveId)
{
	ASSERT_SLAVE_ID_VALID(slaveId);
	return readByte(MPU6050_RA_I2C_SLV0_ADDR + slaveId * 3);
}

void MPU6050::setSlaveAddress(SlaveId slaveId, uint8_t address)
{
	ASSERT_SLAVE_ID_VALID(slaveId);
	I2Cdev::writeByte(devAddr, MPU6050_RA_I2C_SLV0_ADDR + slaveId * 3, address);
}

uint8_t MPU6050::getSlaveRegister(SlaveId slaveId)
{
	ASSERT_SLAVE_ID_VALID(slaveId);
	return readByte(MPU6050_RA_I2C_SLV0_REG + slaveId * 3);
}

void MPU6050::setSlaveRegister(SlaveId slaveId, uint8_t reg)
{
	ASSERT_SLAVE_ID_VALID(slaveId);
	I2Cdev::writeByte(devAddr, MPU6050_RA_I2C_SLV0_REG + slaveId * 3, reg);
}

bool MPU6050::getSlaveEnabled(SlaveId slaveId)
{
	ASSERT_SLAVE_ID_VALID(slaveId);
	return readBit(MPU6050_RA_I2C_SLV0_CTRL + slaveId * 3, MPU6050_I2C_SLV_EN_BIT);
}

void MPU6050::setSlaveEnabled(SlaveId slaveId, bool enabled)
{
	ASSERT_SLAVE_ID_VALID(slaveId);
	I2Cdev::writeBit(devAddr, MPU6050_RA_I2C_SLV0_CTRL + slaveId * 3, MPU6050_I2C_SLV_EN_BIT, enabled);
}

bool MPU6050::getSlaveWordByteSwap(SlaveId slaveId)
{
	ASSERT_SLAVE_ID_VALID(slaveId);
	return readBit(MPU6050_RA_I2C_SLV0_CTRL + slaveId * 3, MPU6050_I2C_SLV_BYTE_SW_BIT);
}

void MPU6050::setSlaveWordByteSwap(SlaveId slaveId, bool enabled)
{
	ASSERT_SLAVE_ID_VALID(slaveId);
	I2Cdev::writeBit(devAddr, MPU6050_RA_I2C_SLV0_CTRL + slaveId * 3, MPU6050_I2C_SLV_BYTE_SW_BIT, enabled);
}

bool MPU6050::getSlaveWriteMode(SlaveId slaveId)
{
	ASSERT_SLAVE_ID_VALID(slaveId);

	return readBit(MPU6050_RA_I2C_SLV0_CTRL + slaveId * 3, MPU6050_I2C_SLV_REG_DIS_BIT);
}

void MPU6050::setSlaveWriteMode(SlaveId slaveId, bool mode)
{
	ASSERT_SLAVE_ID_VALID(slaveId);

	I2Cdev::writeBit(devAddr, MPU6050_RA_I2C_SLV0_CTRL + slaveId * 3, MPU6050_I2C_SLV_REG_DIS_BIT, mode);
}

bool MPU6050::getSlaveWordGroupOffset(SlaveId slaveId)
{
	ASSERT_SLAVE_ID_VALID(slaveId);

	return readBit(MPU6050_RA_I2C_SLV0_CTRL + slaveId * 3, MPU6050_I2C_SLV_GRP_BIT);
}

void MPU6050::setSlaveWordGroupOffset(SlaveId slaveId, bool enabled)
{
	ASSERT_SLAVE_ID_VALID(slaveId);

	I2Cdev::writeBit(devAddr, MPU6050_RA_I2C_SLV0_CTRL + slaveId * 3, MPU6050_I2C_SLV_GRP_BIT, enabled);
}

uint8_t MPU6050::getSlaveDataLength(SlaveId slaveId)
{
	ASSERT_SLAVE_ID_VALID(slaveId);

	return readBits(MPU6050_RA_I2C_SLV0_CTRL + slaveId * 3, MPU6050_I2C_SLV_LEN_BIT, MPU6050_I2C_SLV_LEN_LENGTH);
}

void MPU6050::setSlaveDataLength(SlaveId slaveId, uint8_t length)
{
	ASSERT_SLAVE_ID_VALID(slaveId);

	I2Cdev::writeBits(devAddr, MPU6050_RA_I2C_SLV0_CTRL + slaveId * 3, MPU6050_I2C_SLV_LEN_BIT,
					  MPU6050_I2C_SLV_LEN_LENGTH, length);
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

void MPU6050::setSlaveOutputByte(SlaveId slaveId, uint8_t data)
{
	ASSERT_SLAVE_ID_VALID(slaveId);
	I2Cdev::writeByte(devAddr, MPU6050_RA_I2C_SLV0_DO + slaveId, data);
}

bool MPU6050::getSlaveDelayEnabled(SlaveId slaveId)
{
	// MPU6050_DELAYCTRL_I2C_SLV4_DLY_EN_BIT is 4, SLV3 is 3, etc.
	ASSERT_SLAVE_ID_VALID(slaveId);
	return readBit(MPU6050_RA_I2C_MST_DELAY_CTRL, slaveId);
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
