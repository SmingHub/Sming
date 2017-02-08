// 
// AM2321 Temperature & Humidity Sensor library for Arduino
//
// The MIT License (MIT)
//
// Copyright (c) 2013 Wang Dong
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "AM2321.h"
#include <Wire.h>

#define I2C_ADDR_AM2321                 (0xB8 >> 1)          //AM2321温湿度计I2C地址
#define PARAM_AM2321_READ                0x03                //读寄存器命令
#define REG_AM2321_HUMIDITY_MSB          0x00                //湿度寄存器高位
#define REG_AM2321_HUMIDITY_LSB          0x01                //湿度寄存器低位
#define REG_AM2321_TEMPERATURE_MSB       0x02                //温度寄存器高位
#define REG_AM2321_TEMPERATURE_LSB       0x03                //温度寄存器低位
#define REG_AM2321_DEVICE_ID_BIT_24_31   0x0B                //32位设备ID高8位

template<int I2CADDR, int COMMAND, int REGADDR, int REGCOUNT>
class DataReader {
protected:
    enum { len = 32 };
    uint8_t buf[len];

protected:
    DataReader() {
        memset(buf, 0, len);
    } 
    bool readRaw() {
        //
        // Wakeup
        //
        Wire.beginTransmission(I2CADDR);
        Wire.endTransmission();
        delayMicroseconds(2000);

        Wire.beginTransmission(I2CADDR);
		Wire.endTransmission();
		delayMicroseconds(2000);

		noInterrupts();
        //
        // Read Command
        //
        Wire.beginTransmission(I2CADDR);
        Wire.write(COMMAND);
        Wire.write(REGADDR);
        Wire.write(REGCOUNT);
        if (Wire.endTransmission() != 0)
        {
        	interrupts();
        	debugf("AM2321: Read request failed");
        	return false;
        }

        //
        // Waiting
        //
        delayMicroseconds(2000);

        //
        // Read
        //
        // COMMAND + REGCOUNT + DATA + CRCLSB + CRCMSB
        if (!Wire.requestFrom(I2CADDR, 2 + REGCOUNT + 2))
        {
        	interrupts();
        	debugf("AM2321: Reading failed");
        	return false;
        }

        int i = 0;
        for (; i < 2 + REGCOUNT; ++i)
            buf[i] = Wire.read();

        unsigned short crc = 0;
        crc  = Wire.read();     //CRC LSB
        crc |= Wire.read() << 8;//CRC MSB

        interrupts();

        if (crc == crc16(buf, i))
            return true;

        debugf("AM2321: CRC check failed: %d, %d", crc, crc16(buf, i));
        return false;
    }

private:
    unsigned short crc16(unsigned char *ptr, unsigned char len) {
        unsigned short crc = 0xFFFF; 
        unsigned char  i   = 0;
        while(len--) {
            crc ^= *ptr++; 
            for(i = 0 ; i < 8 ; i++) {
                if(crc & 0x01) {
                    crc >>= 1;
                    crc  ^= 0xA001; 
                }
                else {
                    crc >>= 1;
                } 
            }
        }
        return crc; 
    }
};

class UidReader : public DataReader<I2C_ADDR_AM2321, PARAM_AM2321_READ, REG_AM2321_DEVICE_ID_BIT_24_31, 4>
{
public:
    unsigned int uid;
public:
    bool read() {
        if(!readRaw()) 
            return false;
        uid  = buf[2] << 24;
        uid += buf[3] << 16;
        uid += buf[4] << 8;
        uid += buf[5];
        return true;
    }
};

class AirConditionReader : public DataReader<I2C_ADDR_AM2321, PARAM_AM2321_READ, REG_AM2321_HUMIDITY_MSB, 4>
{
public:
    unsigned int humidity;
    int temperature;
public:
    bool read() {
        if(!readRaw()) 
            return false;
        humidity     = buf[2] << 8;
        humidity    += buf[3];
        temperature  = buf[4] << 8;
        temperature += buf[5];
        return true;
    }
};


AM2321::AM2321() {
    temperature = 0;
    humidity    = 0;
}

void AM2321::begin()
{
	Wire.begin();
}

uint32_t AM2321::uid() {
    UidReader reader;
    if (reader.read())
        return reader.uid;
    return -1;
}


bool AM2321::available() {
    return !(temperature == 0 && humidity == 0);
}

bool AM2321::read() {
    AirConditionReader reader;
    if (reader.read()) {
        temperature = reader.temperature;
        humidity = reader.humidity;
        return true;
    }
    return false;
}
//
// END OF FILE
//
