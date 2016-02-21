/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _APA102_H_
#define _APA102_H_

typedef struct {
    //uint8_t br;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} col_t;


class APA102 {
public:
    APA102(uint16_t n);
    ~APA102(void);

    void begin(void);
    void begin(uint16_t prediv, uint8_t div);
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

private:
    virtual void SPI_transfer(uint8_t * data, uint8_t count);
};



class APA102Soft : public APA102 {
public:
    APA102Soft(uint16_t n, SPISoft *_ptr);
    
    void begin(void);
    void begin(uint16_t prediv, uint8_t div);       // unused function
    void end(void);
    
private:
    SPISoft *pSPI;
    void SPI_transfer(uint8_t * data, uint8_t count);
};

//extern SPIClass SPI;

#endif /* _APA102_H_ */
