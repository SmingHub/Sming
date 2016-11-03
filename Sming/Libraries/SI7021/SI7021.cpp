
#include "Arduino.h"
#include "SI7021.h"

#define I2C_ADDR 0x40

// I2C commands
#define RH_READ             0xE5 
#define TEMP_READ           0xE3 
#define POST_RH_TEMP_READ   0xE0 
#define RESET               0xFE 
#define USER1_READ          0xE7 
#define USER1_WRITE         0xE6
#define CRC8_POLYNOMINAL    0x13100 /* CRC8 polynomial for 16bit CRC8 x^8 + x^5 + x^4 + 1 */

// compound commands
byte UID1_READ[]      ={ 0xFA, 0x0F };
byte UID2_READ[]      ={ 0xFC, 0xC9 };

bool _si_exists = false;

SI7021::SI7021() {
}

bool SI7021::begin() {

	Wire.begin();
    Wire.beginTransmission(I2C_ADDR);

    if (Wire.endTransmission() == 0) {
        _si_exists = true;
        softReset();
    }
    return _si_exists;
}

bool SI7021::sensorExists() {
    return _si_exists;
}


void SI7021::softReset(void)
{
  if (_si_exists != true)
  {
    begin();
  }

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(RESET);
  Wire.endTransmission();
  delay(15);
}

int SI7021::getTemperature() {
    byte tempbytes[2];
    _command(TEMP_READ, tempbytes);
    long tempraw = (long)tempbytes[0] << 8 | tempbytes[1];
    if (_checkCRC8(tempraw) != tempbytes[2]){
    	return 99998;
    }
    return ((17572 * tempraw) >> 16) - 4685;
}

int SI7021::_getTemperaturePostHumidity() {
    byte tempbytes[2];
    _command(POST_RH_TEMP_READ, tempbytes);
    long tempraw = (long)tempbytes[0] << 8 | tempbytes[1];
    return ((17572 * tempraw) >> 16) - 4685;
}


unsigned int SI7021::getHumidityPercent() {
    byte humbytes[2];
    _command(RH_READ, humbytes);
    long humraw = (long)humbytes[0] << 8 | humbytes[1];
    if (_checkCRC8(humraw) != humbytes[2]){
    	return 99999;
    }
    return ((125 * humraw) >> 16) - 6;
}


uint8_t SI7021::_checkCRC8(uint16_t data)
{
  for (uint8_t bit = 0; bit < 16; bit++){
    if (data & 0x8000){
      data =  (data << 1) ^ CRC8_POLYNOMINAL;
    }
    else{
      data <<= 1;
    }
  }
  data >>= 8;
  return data;
}


void SI7021::_command(byte cmd, byte * buf ) {
    _writeReg(&cmd, sizeof cmd);
    delay(25); //for ESP8266
    _readReg(buf, 3); // 1 - data, 2 - data, 3 - CRC
}

void SI7021::_writeReg(byte * reg, int reglen) {
    Wire.beginTransmission(I2C_ADDR);
    for(int i = 0; i < reglen; i++) {
        reg += i;
        Wire.write(*reg); 
    }
    Wire.endTransmission();
}

int SI7021::_readReg(byte * reg, int reglen) {
    Wire.requestFrom(I2C_ADDR, reglen);
    while(Wire.available() < reglen) {
    }
    for(int i = 0; i < reglen; i++) { 
        reg[i] = Wire.read(); 
    }
    return 1;
}


//**************************************************************************

//note this has crc bytes embedded, per datasheet, so provide 12 byte buf
int SI7021::getUIDBytes(byte * buf) {
    _writeReg(UID1_READ, sizeof UID1_READ);
    _readReg(buf, 6);
 
    _writeReg(UID2_READ, sizeof UID2_READ);
    _readReg(buf + 6, 6);
    
    // could verify crc here and return only the 8 bytes that matter
    return 1;
}

int SI7021::getDeviceId() {
    byte uid[12];
    getUIDBytes(uid);
    int id = uid[6];
    return id;
}

void SI7021::setHeater(bool on) {
    byte userbyte;
    if (on) {
        userbyte = 0x3E;
    } else {
        userbyte = 0x3A;
    }
    byte userwrite[] = {USER1_WRITE, userbyte};
    _writeReg(userwrite, sizeof userwrite);
}

//**************************************************************************

// get humidity, then get temperature reading from humidity measurement, calculate Dew Points
struct si7021_env SI7021::getHumidityAndTemperature() {
    si7021_env env       = {0, 0, 0};
    env.humidityPercent  = getHumidityPercent();
    env.temperature      = _getTemperaturePostHumidity();
    env.error_crc 		 = 0;
    if (env.humidityPercent == 99999){
    	env.error_crc    = 1;
    }
    return env;
}

// get temperature only
struct si7021_olt SI7021::getTemperatureOlt() {
    si7021_olt olt       = {0, 0};
    olt.temperature      = getTemperature();
    olt.error_crc        = 0;
    if (olt.temperature == 99998){
    	olt.error_crc    = 1;
    }
    return olt;
}

