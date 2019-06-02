/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 * APA102 library by HappyCodingRobot@github.com
 ****/

#ifndef _APA102_H_
#define _APA102_H_

#include <SPIBase.h>
#include <SPISettings.h>

typedef struct {
    //uint8_t br;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} col_t;


class APA102 {
public:
    APA102(uint16_t n);
    APA102(uint16_t n, SPIBase & spiRef);
    ~APA102(void);

    void begin(void);
    void begin(SPISettings & mySettings);
    void end(void);
    
    /* send data buffer to LEDs, including start & stop sequences */
    void show(void);
    void show(int16_t SPos);
    
    /* clear data buffer */
    void clear(void);
    
    /* set pixel color */
    void setPixel(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
    void setPixel(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t br);
    void setPixel(uint16_t n, col_t* p);
    void setAllPixel(uint8_t r, uint8_t g, uint8_t b);
    //void LEDsetAllPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t br);
    void setAllPixel(col_t*);
    
    /* set global LED brightness level */
    void setBrightness(uint8_t);
    /* get global LED brightness level */
    uint8_t getBrightness(void);
    
    
    /* send start sequence */
    void sendStart(void);
    /* send stop sequence */
    void sendStop(void);
    /* direct write single LED data */
    void directWrite(uint8_t r, uint8_t g, uint8_t b, uint8_t br);

protected:
    uint16_t numLEDs;
    uint8_t *LEDbuffer;
    uint8_t brightness;                 // global brightness 0..31 -> 0..100%
    
    SPISettings SPI_APA_Settings = SPISettings(4000000, MSBFIRST, SPI_MODE3);
    SPIBase & pSPI;

private:
};

#endif /* _APA102_H_ */
