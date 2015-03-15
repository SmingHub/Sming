#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>

#define __CS 10
#define __DC 9
#define __RST 14

// Color definitions
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF

float sin_d[] = { 
  0,0.17,0.34,0.5,0.64,0.77,0.87,0.94,0.98,1,0.98,0.94,
  0.87,0.77,0.64,0.5,0.34,0.17,0,-0.17,-0.34,-0.5,-0.64,
  -0.77,-0.87,-0.94,-0.98,-1,-0.98,-0.94,-0.87,-0.77,
  -0.64,-0.5,-0.34,-0.17 };
float cos_d[] = { 
  1,0.98,0.94,0.87,0.77,0.64,0.5,0.34,0.17,0,-0.17,-0.34,
  -0.5,-0.64,-0.77,-0.87,-0.94,-0.98,-1,-0.98,-0.94,-0.87,
  -0.77,-0.64,-0.5,-0.34,-0.17,0,0.17,0.34,0.5,0.64,0.77,
  0.87,0.94,0.98};
float d = 10;
float px[] = { 
  -d,  d,  d, -d, -d,  d,  d, -d };
float py[] = { 
  -d, -d,  d,  d, -d, -d,  d,  d };
float pz[] = { 
  -d, -d, -d, -d,  d,  d,  d,  d };

float p2x[] = {
  0,0,0,0,0,0,0,0};
float p2y[] = {
  0,0,0,0,0,0,0,0};

int r[] = {
  0,0,0};


TFT_ILI9163C tft = TFT_ILI9163C(__CS, __DC, __RST);

void setup() {
  tft.begin();
  #if defined(__MK20DX128__) || defined(__MK20DX256__)
  tft.setBitrate(24000000);
  #endif
}

void loop(){
  tft.fillScreen(WHITE);
  r[0]=r[0]+1;
  r[1]=r[1]+1;
  if (r[0] == 36) r[0] = 0;
  if (r[1] == 36) r[1] = 0;
  if (r[2] == 36) r[2] = 0;
  for (int i=0;i<8;i++)
  {
    float px2 = px[i];
    float py2 = cos_d[r[0]]*py[i] - sin_d[r[0]]*pz[i];
    float pz2 = sin_d[r[0]]*py[i] + cos_d[r[0]]*pz[i];

    float px3 = cos_d[r[1]]*px2 + sin_d[r[1]]*pz2;
    float py3 = py2;
    float pz3 = -sin_d[r[1]]*px2 + cos_d[r[1]]*pz2;

    float ax = cos_d[r[2]]*px3 - sin_d[r[2]]*py3;
    float ay = sin_d[r[2]]*px3 + cos_d[r[2]]*py3;
    float az = pz3-190;

    p2x[i] = ((tft.width())/2)+ax*500/az;
    p2y[i] = ((tft.height())/2)+ay*500/az;
  }
  for (int i=0;i<3;i++) {
    tft.drawLine(p2x[i],p2y[i],p2x[i+1],p2y[i+1],BLACK);
    tft.drawLine(p2x[i+4],p2y[i+4],p2x[i+5],p2y[i+5],BLACK);
    tft.drawLine(p2x[i],p2y[i],p2x[i+4],p2y[i+4],BLACK);
  }   
  tft.drawLine(p2x[3],p2y[3],p2x[0],p2y[0],BLACK);
  tft.drawLine(p2x[7],p2y[7],p2x[4],p2y[4],BLACK);
  tft.drawLine(p2x[3],p2y[3],p2x[7],p2y[7],BLACK);
  delay(15);
}