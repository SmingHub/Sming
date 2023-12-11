#include <SmingCore.h>
#include <CommandProcessing/Utils.h>

#include <ArduCamCommand.h>

#include <Libraries/ArduCAM/ArduCAM.h>
#include <Libraries/ArduCAM/ov2640_regs.h>

#include <Libraries/ArduCAM/ArduCAMStream.h>
#include <Services/HexDump/HexDump.h>
#include <Data/Stream/MultipartStream.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

#define CAM_SCL 5
#define CAM_SDA 4

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
#define CAM_SCLK 14 // HW SPI pins - don't change
#define CAM_MOSI 13
#define CAM_MISO 12

#define CAM_CS 16 // this pins are free to change

namespace
{
HttpServer server;

CommandProcessing::Handler commandHandler;

ArduCAM myCAM(OV2640, CAM_CS);
ArduCamCommand arduCamCommand(myCAM, commandHandler);

SPISettings spiSettings(20000000, MSBFIRST, SPI_MODE0);

/*
 * set the command handler -> ArduCamCommand handler
 */
void startApplicationCommand()
{
	arduCamCommand.initCommand();
}

bool processTelnetInput(TcpClient& client, char* data, int size)
{
	return client.sendString(commandHandler.processNow(data, size));
}

TcpServer telnetServer(processTelnetInput);

/*
 * initCam()
 *
 * Initialize I2C, SPI Bus and check if the camera is there
 * Initialize the camera for JPEG 320x240
 *
 */
void initCam()
{
	uint8_t vid, pid = 0;

	Serial.printf("ArduCAM init!");

	// initialize I2C
	Wire.pins(CAM_SDA, CAM_SCL);
	Wire.begin();

	//Check if the camera module type is OV2640
	myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
	myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
	if((vid != 0x26) || (pid != 0x42)) {
		Serial.println(_F("Can't find OV2640 module!"));
		Serial << "vid = [" << String(vid, HEX) << "], pid = [" << String(pid, HEX) << "]" << endl;
	} else {
		Serial.println(_F("OV2640 detected"));
	}

	// initialize SPI:
	pinMode(CAM_CS, OUTPUT);
	digitalWrite(CAM_CS, HIGH);
	SPI.begin();
	SPI.beginTransaction(spiSettings);

	//Check if the ArduCAM SPI bus is OK
	myCAM.write_reg(ARDUCHIP_TEST1, 0x55);

	uint8_t temp = myCAM.read_reg(ARDUCHIP_TEST1);
	if(temp != 0x55) {
		Serial.println(_F("SPI interface Error!"));
		while(1) {
			// loop forever
		}
	} else {
		Serial.println(_F("SPI interface OK!"));
	}

	// init CAM
	Serial.println(_F("Initialize the OV2640 module"));
	myCAM.set_format(JPEG);
	myCAM.InitCAM();
}

void startCapture()
{
	//  Serial.printf("startCapture()\n");
	myCAM.clear_fifo_flag();
	myCAM.start_capture();
}

/*
 * default http handler to check if server is up and running
 */

void onIndex(HttpRequest& request, HttpResponse& response)
{
	response.sendFile(_F("index.html"));
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();

	if(file[0] == '.')
		response.code = HTTP_STATUS_FORBIDDEN;
	else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onCamSetup(HttpRequest& request, HttpResponse& response)
{
	String size, type;

	if(request.method == HTTP_POST) {
		type = request.getPostParameter("type");
		debugf("set type %s", type.c_str());
		arduCamCommand.setType(type);

		size = request.getPostParameter("size");
		debugf("set size %s", size.c_str());
		arduCamCommand.setSize(size);
	}

	response.sendString("OK");
}

/*
 * http request to capture and send an image from the camera
 * uses actual setting set by ArdCammCommand Handler
 */
void onCapture(HttpRequest& request, HttpResponse& response)
{
	Serial.printf("perform onCapture()\r\n");

	// TODO: use request parameters to overwrite camera settings
	// setupCamera(camSettings);
	myCAM.clear_fifo_flag();
	myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);

	// get the picture
	OneShotFastMs timer;
	startCapture();
	Serial << _F("onCapture() startCapture() ") << timer.elapsedTime() << endl;

	ArduCAMStream* stream = new ArduCAMStream(&myCAM);

	const char* contentType = arduCamCommand.getContentType();

	if(stream->dataReady()) {
		response.headers[HTTP_HEADER_CONTENT_LENGTH] = String(stream->available());
		response.sendDataStream(stream, contentType);
	}

	Serial << _F("onCapture() process Stream ") << timer.elapsedTime() << endl;
}

MultipartStream::BodyPart snapshotProducer()
{
	MultipartStream::BodyPart result;

	startCapture();
	ArduCAMStream* camStream = new ArduCAMStream(&myCAM);
	result.stream = camStream;

	result.headers = new HttpHeaders();
	(*result.headers)[HTTP_HEADER_CONTENT_TYPE] = toString(MIME_JPEG);

	return result;
}

void onStream(HttpRequest& request, HttpResponse& response)
{
	Serial.println(_F("perform onCapture()"));

	// TODO: use request parameters to overwrite camera settings
	// setupCamera(camSettings);
	myCAM.clear_fifo_flag();
	myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);

	auto stream = new MultipartStream(snapshotProducer);
	response.sendDataStream(stream, F("multipart/x-mixed-replace; boundary=") + stream->getBoundary());
}

void onFavicon(HttpRequest& request, HttpResponse& response)
{
	response.code = HTTP_STATUS_NOT_FOUND;
}

/*
 * start http and telnet server
 * telnet can be used to configure camera settings
 * using ArdCammCommand handler
 */
void startServers()
{
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/cam/set", onCamSetup);
	server.paths.set("/cam/capture", onCapture);
	server.paths.set("/stream", onStream);
	server.paths.set("/favicon.ico", onFavicon);
	server.paths.setDefault(onFile);

	Serial.println(_F("\r\n=== WEB SERVER STARTED ==="));
	Serial.println(WifiStation.getIP());
	Serial.println(_F("==============================\r\n"));

	telnetServer.listen(23);
	Serial.println(_F("\r\n=== TelnetServer SERVER STARTED ==="));
	Serial.println(_F("==============================\r\n"));
}

// Will be called when station is fully operational
void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	startServers();
}

} // namespace

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	// Process commands from serial
	commandHandler.setVerbose(true);
	CommandProcessing::enable(commandHandler, Serial);

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	WifiEvents.onStationGotIP(gotIP);

	// setup the ArduCAM
	initCam();

	// set command handlers for cam
	startApplicationCommand();
}
