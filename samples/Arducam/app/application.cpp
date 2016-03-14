#include <user_config.h>
#include <SmingCore/SmingCore.h>
//#include <SmingCore/Network/WebConstants.h>
#include <SmingCore/Network/TelnetServer.h>
//#include <SmingCore/Debug.h>
//#include <Libraries/ArduCAM/ArduCAM.h>
//#include <Libraries/ArduCAM/ov2640_regs.h>

//#include "CamSettings.h"
#include <ArduCamCommand.h>

#include <Libraries/ArduCam/ArduCAM.h>
#include <Libraries/ArduCAM/ov2640_regs.h>

#include <Libraries/ArduCam/ArduCAMStream.h>
#include <Services/HexDump/HexDump.h>


// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif


#define CAM_SCL		5
#define CAM_SDA		4

/*
 * Hardware SPI mode:
 * GND      (GND)         GND
 * VCC      (VCC)         3.3v
 * D0       (CLK)         GPIO14
 * D1       (MOSI)        GPIO13
 * CS       (CS)          GPI015
 * RES      (RESET)       GPIO16
 * DC       (DC)          GPIO2
 */
#define CAM_SCLK 	14  // HW SPI pins - dont change
#define CAM_MOSI 	13
#define CAM_MISO	12

#define CAM_CS   	16	// this pins are free to change

uint32_t startTime;

TelnetServer telnet;
HttpServer server;

HexDump hdump;


ArduCAM myCAM(OV2640,CAM_CS);

ArduCamCommand arduCamCommand(&myCAM);

/*
 * set the command handler -> ArduCamCommand handler
 */
void startApplicationCommand()
{
	arduCamCommand.initCommand();
}

/*
 * initCam()
 *
 * Initalize I2C, SPI Bus and check if the cammera is there
 * Initialize the camera for JPEG 320x240
 *
 */
void initCam() {
	uint8_t vid,pid = 0;

	Serial.printf("ArduCAM init!");

	// initialize I2C
	Wire.pins(CAM_SCL, CAM_SDA);
	Wire.begin();

	//Check if the camera module type is OV2640
	myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
	myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
	if((vid != 0x26) || (pid != 0x42)) {
		Serial.println("Can't find OV2640 module!");
		Serial.printf("vid = [%X]  pid = [%X]\n", vid, pid);
	}
	else
	  	Serial.println("OV2640 detected");

	// initialize SPI:
    pinMode(CAM_CS, OUTPUT);
	digitalWrite(CAM_CS, HIGH);
	SPI.begin();
	SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));


	//Check if the ArduCAM SPI bus is OK
	myCAM.write_reg(ARDUCHIP_TEST1, 0x55);

	uint8_t temp = myCAM.read_reg(ARDUCHIP_TEST1);
	if (temp != 0x55) {
		Serial.println("SPI interface Error!");
		while (1);
	} else {
		Serial.println("SPI interface OK!");
	}

	// init CAM
	Serial.println("Initialize the OV2640 module");
	myCAM.set_format(JPEG);
	myCAM.InitCAM();
}


void startCapture(){
//  Serial.printf("startCapture()\n");
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
}

/*
 * default http handler to check if server is up and running
 */

void onIndex(HttpRequest &request, HttpResponse &response)
{

	Serial.printf("perform onIndex()\n");

	String msg = "<html>ArduCAM Ready for <a href=\"./capture\">capture</a></html>";

	response.setContentType("text/html");
	response.sendString(msg);

	hdump.resetAddr();
	hdump.print((unsigned char*)msg.c_str(), msg.length());

}

/*
 * http request to capture and send an image from the cammera
 * uses actual setting set by ArdCammCommand Handler
 */
void onCapture(HttpRequest &request, HttpResponse &response) {

	Serial.printf("perform onCapture()\r\n");

	// TODO: use request parameters to overwrite camera settings
	// setupCamera(camSettings);
	myCAM.clear_fifo_flag();
	myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);

	// get the picture
	startTime = millis();
	startCapture();
	Serial.printf("onCapture() startCapture() %d ms\r\n", millis() - startTime);

	ArduCAMStream *stream = new ArduCAMStream(&myCAM);

	const char * contentType = arduCamCommand.getContentType();

	if (stream->dataReady()) {
		response.setHeader("Content Lenght", String(stream->available()));
		response.sendDataStream(stream, contentType);
	}

	Serial.printf("onCapture() process Stream %d ms\r\n", millis() - startTime);
}

void onStream(HttpRequest &request, HttpResponse &response) {

	Serial.printf("perform onCapture()\r\n");

	// TODO: use request parameters to overwrite camera settings
	// setupCamera(camSettings);
	myCAM.clear_fifo_flag();
	myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);

	// get the picture
	startTime = millis();
	startCapture();
	Serial.printf("onCapture() startCapture() %d ms\r\n", millis() - startTime);

	response.setContentType("Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n");
	response.sendString("HTTP/1.1 200 OK\r\n");


	while (1) {
		startCapture();
		ArduCAMStream *stream = new ArduCAMStream(&myCAM);

		if (stream->dataReady()) {
			response.sendString("--frame\r\n");
			response.sendDataStream(stream, "Content-Type: image/jpeg\r\n\r\n");
		}
	}
}

void onFavicon(HttpRequest &request, HttpResponse &response) {
	response.notFound();
}

/*
 * start http and telnet server
 * telnet can be used to configure cammera settings
 * unsing ArdCammCommand handler
 */
void StartServers()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/capture", onCapture);
//	server.addPath("/stream", onStream);
	server.addPath("/favicon.ico", onFavicon);
	server.setDefaultHandler(onIndex);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");


	telnet.listen(23);
	telnet.enableDebug(true);

	Serial.println("\r\n=== TelnetServer SERVER STARTED ===");
	Serial.println("==============================\r\n");
}


// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");
	StartServers();
}


void init()
{

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	Debug.setDebug(Serial);

	Serial.systemDebugOutput(true); // Enable debug output to serial
	Serial.commandProcessing(true);

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk);

	// setup the ArduCAM
	initCam();

	// set command handlers for cam
	startApplicationCommand();

}
