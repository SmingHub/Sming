

/* This library for Digital Light sensor BH1750FVI

 use I2C Communication protocal , SDA,SCL Are required

  to interface with this sensor

  pin configuration :

  VCC >>> 3.3V
  SDA >>> A4
  SCL >>> A5
  ADDR >> A3 "Optional"
  GND >>> gnd

  written By : Mohannad Rawashdeh
  www.genotronex.com
  */

#ifndef BH1750FVI_h
#define BH1750FVI_h

#include "Arduino.h"
#include "Wire.h"

#define BH1750FVI_ADDRESS_LOW 0x23 // Device address when Address pin LOW
#define BH1750FVI_ADDRESS_HIGH 0x5C // Device address when Address pin HIGH

//all command here taken from Data sheet OPECODE Table page 5
#define BH1750_Power_Down	0x00
#define BH1750_Power_On		0x01
#define BH1750_Reset		0x07

#define BH1750_Continuous_H_resolution_Mode  0x10
#define BH1750_Continuous_H_resolution_Mode2  0x11
#define BH1750_Continuous_L_resolution_Mode  0x13
#define BH1750_OneTime_H_resolution_Mode  0x20
#define BH1750_OneTime_H_resolution_Mode2  0x21
#define BH1750_OneTime_L_resolution_Mode  0x23//As well as address value

class BH1750FVI
{
public:
    BH1750FVI(byte address);
    bool begin(void);
    void sleep(void);
    void setMode(uint8_t MODE);
    void reset(void);
    uint16_t getLightIntensity(void);

private:
    bool I2CWriteTo(uint8_t DataToSend);
    byte address_value;
};
#endif



