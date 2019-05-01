
#ifndef si7021_h
#define si7021_h

#include <inttypes.h>
#include <Wire.h>

typedef struct si7021_env {
	int temperature;
	unsigned int humidityPercent;
	int error_crc;
} si7021_env;

typedef struct si7021_olt {
	int temperature;
	int error_crc;
} si7021_olt;


class SI7021
{
  public:
    SI7021();

    bool begin();
    bool sensorExists();
    void softReset(void);

    int getTemperature();
    unsigned int getHumidityPercent();

    struct si7021_env getHumidityAndTemperature();
    struct si7021_olt getTemperatureOlt();

    int getUIDBytes(byte * buf);
    int getDeviceId();
    void setHeater(bool on);

  private:
    void _command(byte cmd, byte * buf );
    void _writeReg(byte * reg, int reglen);
    int _readReg(byte * reg, int reglen);
    int _getTemperaturePostHumidity();
    uint8_t  _checkCRC8(uint16_t data);
};

#endif

