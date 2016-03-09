#ifndef MT9M001_REGS_H
#define MT9M001_REGS_H
#include "ArduCAM.h"
//#include <avr/pgmspace.h>

const struct sensor_reg MT9M001_QVGA_30fps[] PROGMEM=
{
		
		{0x03, 240},//Row Size
		{0x04, 639},//Col Size

    {0xff, 0xffff }
};




#endif //MT9M001_REGS_H