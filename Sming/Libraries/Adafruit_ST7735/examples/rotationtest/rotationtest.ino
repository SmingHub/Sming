/***************************************************
  This is a library for the Adafruit 1.8" SPI display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>


// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     10
#define TFT_RST    9  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC     8

// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Option 2: use any pins but a little slower!
#define TFT_SCLK 13   // set these to be whatever pins you like!
#define TFT_MOSI 11   // set these to be whatever pins you like!
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void setup(void) {
  Serial.begin(9600);
  Serial.print("Hello! Adafruit ST7735 rotation test");

  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  // Use this initializer (uncomment) if you're using a 1.44" TFT
  //tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab

  Serial.println("init");

  tft.setTextWrap(false); // Allow text to run off right edge
  tft.fillScreen(ST7735_BLACK);

  Serial.println("This is a test of the rotation capabilities of the TFT library!");
  Serial.println("Press <SEND> (or type a character) to advance");
}

void loop(void) {
  rotateLine();
  rotateText();
  rotatePixel();
  rotateFastline();
  rotateDrawrect();
  rotateFillrect();
  rotateDrawcircle();
  rotateFillcircle();
  rotateTriangle();
  rotateFillTriangle();
  rotateRoundRect();
  rotateFillRoundRect();
  rotateChar();
  rotateString();
}

void rotateText() {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.setCursor(0, 30);
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(1);
    tft.println("Hello World!");
    tft.setTextColor(ST7735_YELLOW);
    tft.setTextSize(2);
    tft.println("Hello World!");
    tft.setTextColor(ST7735_GREEN);
    tft.setTextSize(3);
    tft.println("Hello World!");
    tft.setTextColor(ST7735_BLUE);
    tft.setTextSize(4);
    tft.print(1234.567);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();
  
    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateFillcircle(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.fillCircle(10, 30, 10, ST7735_YELLOW);

    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateDrawcircle(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawCircle(10, 30, 10, ST7735_YELLOW);
 
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();
  
    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateFillrect(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.fillRect(10, 20, 10, 20, ST7735_GREEN);
 
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateDrawrect(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawRect(10, 20, 10, 20, ST7735_GREEN);
 
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateFastline(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawFastHLine(0, 20, tft.width(), ST7735_RED);
    tft.drawFastVLine(20, 0, tft.height(), ST7735_BLUE);

    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateLine(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawLine(tft.width()/2, tft.height()/2, 0, 0, ST7735_RED);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotatePixel(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawPixel(10,20, ST7735_WHITE);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateTriangle(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawTriangle(20, 10, 10, 30, 30, 30, ST7735_GREEN);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateFillTriangle(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.fillTriangle(20, 10, 10, 30, 30, 30, ST7735_RED);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateRoundRect(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawRoundRect(20, 10, 25, 15, 5, ST7735_BLUE);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateFillRoundRect(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.fillRoundRect(20, 10, 25, 15, 5, ST7735_CYAN);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateChar(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawChar(25, 15, 'A', ST7735_WHITE, ST7735_WHITE, 1);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateString(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.setCursor(8, 25);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Adafruit Industries");
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

