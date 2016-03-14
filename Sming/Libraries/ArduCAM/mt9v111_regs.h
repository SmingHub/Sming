#ifndef MT9V111_REGS_H
#define MT9V111_REGS_H
#include "ArduCAM.h"
#include <Wire.h>

#define I2C_SLAVE_ADDRESS 0xB8


const struct sensor_reg MT9V111_QVGA[] PROGMEM=
{
	{0x01, 	0x04},		//REG=4 Sensor Core Register Access
  {64, 	488 }, 		// anti-eclipse Vref
	{47, 	63408},		//set for Fmclk>13.5MHz
	{2, 	11},
	{3,		487},
	{4, 	646},
  {0x06, 	70 }, 		//vertical blank, 
  {32, 	0x4000},
  
  {0x01, 	1}, 		//REG=1	IFP Register Access
  {0x08, 	0xd802},	//RGB mode
  {58, 	0x0001},
  {165, 	0x8000}, 	// [QVGA 1:1 zoom]
  {166, 	0x8280}, 	// 
  {167, 	0x8140}, 	// 
  {168, 	0x8000}, 	// 
  {169, 	0x81e0}, 	// 
	{170, 	0x00f0}, 	//
	
  	//[Auto Lens Correction Setup]
	{0x2B, 0x0022}, 	//(36) GREEN1_GAIN_REG	
	{0x2C, 0x003E}, 	//(48) BLUE_GAIN_REG
	{0x2D, 0x0020}, 	//(53) RED_GAIN_REG	
	{0x2E, 0x0066}, 	//(36) GREEN2_GAIN_REG	
	{0x02, 0x0000}, 	//(18) BASE_MATRIX_SIGNS
	{0x03, 0x3923}, 	//(11) BASE_MATRIX_SCALE_K1_K5
	{0x04, 0x0724}, 	//(10) BASE_MATRIX_SCALE_K6_K9
	{0x06, 0xF00C}, 	//(14) MODE_CONTROL R6[14]:enable AEs
	{0x09, 0x0080}, 	//(3) BASE_MATRIX_COEF_K1
	{0x0A, 0x0000}, 	//(2) BASE_MATRIX_COEF_K2
	{0x0B, 0x0000}, 	//(2) BASE_MATRIX_COEF_K3
	{0x0C, 0x0000}, 	//(1) BASE_MATRIX_COEF_K4	
	{0x0D, 0x0080}, 	//(3) BASE_MATRIX_COEF_K5
	{0x0E, 0x0000}, 	//(2) BASE_MATRIX_COEF_K6
	{0x0F, 0x0000}, 	//(2) BASE_MATRIX_COEF_K7
	{0x10, 0x0000}, 	//(2) BASE_MATRIX_COEF_K8	
	{0x11, 0x0080}, 	//(3) BASE_MATRIX_COEF_K9
	{0x15, 0x0000}, 	//(21) DELTA_COEFS_SIGNS
	{0x16, 0x0000}, 	//(3) DELTA_MATRIX_COEF_D1
	{0x17, 0x0000}, 	//(2) DELTA_MATRIX_COEF_D2
	{0x18, 0x0000}, 	//(2) DELTA_MATRIX_COEF_D3
	{0x19, 0x0000}, 	//(1) DELTA_MATRIX_COEF_D4
	{0x1A, 0x0000}, 	//(3) DELTA_MATRIX_COEF_D5
	{0x1B, 0x0000}, 	//(2) DELTA_MATRIX_COEF_D6
	{0x1C, 0x0000}, 	//(2) DELTA_MATRIX_COEF_D7
	{0x1D, 0x0000}, 	//(2) DELTA_MATRIX_COEF_D8
	{0x1E, 0x0000}, 	//(4) DELTA_MATRIX_COEF_D9
	{0x25, 0x0514}, 	//(1) AWB_SPEED_SATURATION
	{0x34, 0x0010}, 	//(5) LUMA_OFFSET
	{0x35, 0xF010}, 	//(10) CLIPPING_LIM_OUT_LUMA
	{0x48, 0x0080}, 	//(1) TEST_PATTERN_GEN
	{0x53, 0x0804}, 	//(10) GAMMA_Y1_Y2	
	{0x54, 0x2010}, 	//(10) GAMMA_Y3_Y4	
	{0x55, 0x6040}, 	//(10) GAMMA_Y5_Y6	
	{0x56, 0xA080}, 	//(10) GAMMA_Y7_Y8	
	{0x57, 0xE0C0}, 	//(10) GAMMA_Y9_Y10

	{0xff, 0xffff},
};

#endif

