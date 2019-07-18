# ArduCAM Library

## Introduction

This is a opensource library for taking high resolution still images and short video clip on Arduino based platforms using ArduCAM's camera moduels. <br>
The camera breakout boards should work with ArduCAM shield before connecting to the Arduino boards. <br>
ArduCAM mini series camera modules like Mini-2MP, Mini-5MP(Plus) can be connected to Arduino boards directly. <br>
In addition to Arduino, the library can be ported to any hardware platforms as long as they have I2C and SPI interface based on this ArduCAM library. <br>

### Now Supported Cameras
-	OV7660		0.3MP
-	OV7670		0.3MP
-	OV7675		0.3MP
-	OV7725		0.3MP
-	MT9V111		0.3MP
-	MT9M112		1.3MP	
-	MT9M001		1.3MP 	
-	MT9D111		2MP
-	OV2640		2MP	JPEG
-	MT9T112		3MP
-	OV3640		3MP
-	OV5642		5MP	JPEG
-	OV5640		5MP JPEG

### Supported MCU Platform
-	Theoretically support all Arduino families
-	Arduino UNO R3			(Tested)
-	Arduino MEGA2560 R3		(Tested)
-	Arduino Leonardo R3		(Tested)
-	Arduino Nano			(Tested)
-	Arduino DUE			(Tested)
-	Arduino Genuion 101		(Tested)
-	Raspberry Pi			(Tested)
-	ESP8266-12			(Tested) (http://www.arducam.com/downloads/ESP8266_UNO/package_ArduCAM_index.json)
*	Feather M0              (Tested with OV5642)

Note: ArduCAM library for ESP8266 is maintained in another repository [ESP8266](https://github.com/ArduCAM/ArduCAM_ESP8266_UNO) using a json board manager script. <br>

### Libraries Structure
The basic libraries are composed by two sub-libraries one is ArduCAM and the other is UTFT4ArduCAM_SPI. These two libraries should be copied right under the libraries of Arduino directory in order to be recognized by the Arduino IDE. <br>
The ArduCAM library is the core library for ArduCAM shields. It contains supported image sensor drivers and user land API functions which issue capture or image data read commands .There is also an example directory inside the ArduCAM library which illustrates most 
function of the ArduCAM shields. The existing examples are plug and play without need to write a single line of code.  <br>
The UTFT4ArduCAM_SPI library is modified version of UTFT which is written by [Henning Karlsen](http://www.henningkarlsen.com/electronics). We ported it to support ArduCAM 
shield with LCD screen. So the UTFT4ArduCAM_SPI library is only needed when using the ArduCAM-LF model. <br>

## How to use
The libraries should be configured before running any examples, or else you will get a compilation error message. <br>

### 1. Edit memorysaver.h file
Open the `memorysaver.h` file in the ArduCAM folder and enable the hardware platform and camera module which matches to your hardware by comment or 
uncomment the macro definition in the file. For example, if you got a ArduCAM-Mini-2MP you 
should uncomment the line `#define OV2640_MINI_2MP` and comment all the other lines. And 
if you got a ArduCAM-Shield-V2 and a OV5642 camera module, you should uncomment the line `#define ARDUCAM_SHIELD_V2` 
and the line `#define OV5642_CAM` then comment other lines. <br>

### 2. Choose correct CS pin for your camera
Open one of the examples, wiring SPI and I2C interface especially CS pins to ArduCAM shield according to the examples.
Hardware and software shuld be consistent to run the examples correctly.

### 3. Upload the examples
In the example folder there are seven sub directories for different ArduCAM models and the host application. 
The `Mini` folder is for ArduCAM-Mini-2MP and ArduCAM-Mini-5MP modules.  <br>
The `Mini_5MP_Plus` folder is for ArduCAM-Mini-5MP-Plus (OV5640/OV5642) modules.  <br>
The `RevC` folder is for ArduCAM-Shield-RevC or ArduCAM-Shield-RevC+ shields.  <br>
The `Shield_V2` folder is for ArduCAM-Shield-V2 shield.  <br>
The `host_app` folder is host capture and display application for all of ArduCAM modules. <br>
The `RaspberryPi` folder is examples used for Raspberry Pi platform, see [more instruction](https://github.com/ArduCAM/Arduino/tree/master/ArduCAM/examples/RaspberryPi). <br>
The `ESP8266` folder is for ArduCAM-ESP8266-UNO board examples for library compatibility. Please try repository [ESP8266](https://github.com/ArduCAM/ArduCAM_ESP8266_UNO) using josn board manager script instead.<br>
Selecting correct COM port and Arduino boards then upload the sketches.  <br>

## Arducam MINI Camera Demo Tutorial for Arduino

[![IMAGE ALT TEXT](https://github.com/UCTRONICS/pic/blob/master/Arducam_MINI_Camera.jpeg)](https://youtu.be/hybQpjwJ4aA  "Arducam MINI Camera Demo Tutorial for Arduino")

## Arducam Camera Shield V2 Demo Tutorial for Arduino

[![IMAGE ALT TEXT](https://github.com/UCTRONICS/pic/blob/master/Arducam_Shield_V2_Camera.jpeg)](https://youtu.be/XMik38TNqGk  "Arducam MINI Camera Demo Tutorial for Arduino")

### 4. How To Connect Bluetooth Module
- Using this demo 

```Bash
https://github.com/ArduCAM/Arduino/blob/master/ArduCAM/examples/mini/ArduCAM_Mini_Video_Streaming_Bluetooth/ArduCAM_Mini_Video_Streaming_Bluetooth.ino
```

 ![Alt text](https://github.com/ArduCAM/Arduino/blob/master/Arducam_Mini_VideoStreaming_BlueTooth.png)

## 5. How to download the Host V2 ?

- For ArduCAM_Host_V2.0_Mac.app, please refer to this link:
https://www.arducam.com/downloads/app/ArduCAM_Host_V2.0_Mac.app.zip

- For ArduCAM_Mini_V2.0_Linux_x86_64bit, Please refer to this link:
https://www.arducam.com/downloads/app/ArduCAM_Mini_V2.0_Linux_x86_64bit.zip
