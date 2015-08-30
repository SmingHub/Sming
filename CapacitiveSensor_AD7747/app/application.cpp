// --------------------------------------
// i2c_scanner (Sming version)
//
// Version 1
//    This program (or code that looks like it)
//    can be found in many places.
//    For example on the Arduino.cc forum.
//    The original author is not know.
// Version 2, Juni 2012, Using Arduino 1.0.1
//     Adapted to be as simple as possible by Arduino.cc user Krodal
// Version 3, Feb 26  2013
//    V3 by louarnold
// Version 4, March 3, 2013, Using Arduino 1.0.3
//    by Arduino.cc user Krodal.
//    Changes by louarnold removed.
//    Scanning addresses changed from 0...127 to 1...119,
//    according to the i2c scanner by Nick Gammon
//    http://www.gammon.com.au/forum/?id=10896
// Version 5, March 28, 2013
//    As version 4, but address scans now to 127.
//    A sensor seems to use address 120.
// Version 6, March 15  2015
//    Sming version by Skurydin Alexey
//
//
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
//

/*
#include "../../Sming/SmingCore/HardwareSerial.h"
#include "../../Sming/SmingCore/Platform/WDT.h"
#include "../../Sming/SmingCore/Timer.h"
#include "../../Sming/SmingCore/Wire.h"
#include "../../Sming/Wiring/WConstants.h"
#include "../include/user_config.h"
#include <SmingCore/SmingCore.h>
*/


#include <user_config.h>
#include <SmingCore/SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif


Timer procTimer;

HttpServer server;
int totalActiveSockets = 0;
float pf;

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendTemplate(tmpl); // this template object will be deleted automatically
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void wsConnected(WebSocket& socket)
{
	totalActiveSockets++;

	// Notify everybody about new connection
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++)
	{
		//clients[i].sendString("New friend arrived! Total: " + String(totalActiveSockets));
		char buf[15];
		os_sprintf(buf,"%f",pf);
		clients[i].sendString(buf);

		Serial.print("Capacitance: ");
		Serial.println(buf);
	}
}

void wsMessageReceived(WebSocket& socket, const String& message)
{
	Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
	String response = "Echo: " + message;
	socket.sendString(response);
}

void wsBinaryReceived(WebSocket& socket, uint8_t* data, size_t size)
{
	Serial.printf("Websocket binary data recieved, size: %d\r\n", size);
}

void wsDisconnected(WebSocket& socket)
{
	totalActiveSockets--;

	// Notify everybody about lost connection
	WebSocketsList &clients = server.getActiveWebSockets();
	for (int i = 0; i < clients.count(); i++)
		clients[i].sendString("We lost our friend :( Total: " + String(totalActiveSockets));
}

void readPeriodically()
{
	Wire.beginTransmission(0x48);   //talking to chip
	Wire.write(byte(0x00));   //status register address
	Wire.endTransmission();
	Wire.requestFrom(0x48, 1);   //request status register data
	int readycap;
	//Serial.println(" Trying read...");   //try read
	readycap = Wire.read();
	if ((readycap & 0x1) == 0)
	{                // ready?
		//Serial.print(system_get_time());
		//Serial.println(" Data Ready");
		//delay(10);
		Wire.beginTransmission(0x48);    //arduino asks for data from ad7747
		Wire.write(0x01);   		     //set address point to capacitive DAC register 1
		Wire.endTransmission();          //pointer is set so now we can read the

		//Serial.print(system_get_time());
		//Serial.println(" Data Incoming");
		//delay(10);
		Wire.requestFrom(0x48, 3,false);   //reads data from cap DAC registers 1-3
		while (Wire.available())
		{
			//Serial.print(system_get_time());
			//Serial.println("  Wire available.");
			unsigned char hi, mid, lo;      //1 byte numbers
			long capacitance;      //will be a 3byte number
			hi = Wire.read();
			mid = Wire.read();
			lo = Wire.read();
			capacitance = (hi << 16) + (mid << 8) + lo - 0x800000;
			pf = (float) capacitance * -1 / (float) 0x800000 * 8.192f;
			//Serial.print(system_get_time());
			//Serial.print(" ");
			//Serial.println(pf, DEC); //prints the capacitance data in decimal through serial port
		}
		//Serial.println();
	}
	//Serial.print(system_get_time());
	//Serial.println(" Loop Done");
	system_soft_wdt_feed();
}

void readAD7747()
{
	// http://opensourceecology.org/w/images/e/ec/Cap_Sensor_Email_Chain_7-11-2014.pdf
	// http://opensourceecology.org/wiki/Paul_Log

	system_soft_wdt_feed();

	Wire.begin();                   //sets up i2c for operation
	Wire.beginTransmission(0x48);
	Wire.write(0xBF);
	Wire.write(0x00);
	Wire.endTransmission();
	delay(4);
	Wire.beginTransmission(0x48);                   // begins write cycle
	Wire.write(0x07);                   //address pointer for cap setup register
	Wire.write(0xA0);                 //b'10100000' found from datasheet page 16
	Wire.endTransmission();                   //ends write cycle
	delay(4); // Wait for data to clock out? I'm not 100% sure why this delay is here (or why it's 4ms)
	Wire.beginTransmission(0x48);                   //begins transmission again
	Wire.write(0x09); //address pointer for capacitive channel excitation register
	Wire.write(0x0E);                   //recommended value from datasheet
	Wire.endTransmission();
	delay(4);
	Wire.beginTransmission(0x48);
	Wire.write(0x0A);           //address pointer for the configuration register
	Wire.write(0x21); 			//b'00100001' for continuous conversion, arbitrary VTF setting, and mid-range capacitive conversion time
	Wire.endTransmission();
	Wire.beginTransmission(0x48);
	Wire.write(0x0B);           //CAP DAC A Register address (Positive pin data)
	Wire.write(0x80);           //b'10111111' for enable Cap DAC A
	Wire.endTransmission();
	Serial.println("Loop will start");   //test to make sure serial connection is working
	//WDT.alive();
	//system_soft_wdt_restart();

	procTimer.initializeMs(200, readPeriodically).start();

}


void scanBus()
{
	byte error, address;
	int nDevices;

	Serial.println("Scanning...");
	//WDT.enable(false);

	nDevices = 0;
	for (address = 1; address < 127; address++)
	{
		// The i2c_scanner uses the return value of
		// the Write.endTransmisstion to see if
		// a device did acknowledge to the address.

		Wire.beginTransmission(address);
		error = Wire.endTransmission();

		if (error == 0)
		{
			Serial.print("I2C device found at address 0x");
			if (address < 16)
				Serial.print("0");
			Serial.print(address, HEX);
			Serial.println("  !");

			nDevices++;
		}
		else if (error == 4)
		{
			Serial.print("Unknow error at address 0x");
			if (address < 16)
				Serial.print("0");
			Serial.println(address, HEX);
		}
	}
	if (nDevices == 0)
		Serial.println("No I2C devices found\n");
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.setDefaultHandler(onFile);

	// Web Sockets configuration
	server.enableWebSockets(true);
	server.setWebSocketConnectionHandler(wsConnected);
	server.setWebSocketMessageHandler(wsMessageReceived);
	server.setWebSocketBinaryHandler(wsBinaryReceived);
	server.setWebSocketDisconnectionHandler(wsDisconnected);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");

}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");
	Serial.println(WifiStation.getIP().toString());
	startWebServer();
	readAD7747();
}

void couldntConnect()
{
	Serial.println("Couldn't connect");
}

void init()
{

	System.setCpuFrequency(eCF_160MHz);

	Serial.systemDebugOutput(false); // Disable debug output
	Serial.print("rst_info.reason: ");
	Serial.println(system_get_rst_info()->reason);
	Serial.print("rst_info.depc ");
	Serial.println(system_get_rst_info()->depc, HEX);
	Serial.print("rst_info.epc1 ");
	Serial.println(system_get_rst_info()->epc1, HEX);
	Serial.print("rst_info.epc2 ");
	Serial.println(system_get_rst_info()->epc2, HEX);
	Serial.print("rst_info.epc3 ");
	Serial.println(system_get_rst_info()->epc3, HEX);
	Serial.print("rst_info.exccause ");
	Serial.println(system_get_rst_info()->exccause, HEX);
	Serial.print("rst_info.excvaddr ");
	Serial.println(system_get_rst_info()->excvaddr, HEX);

	//WDT.enable(false); // First (but not the best) option: fully disable watch dog timer
	system_soft_wdt_stop();

	// You can change pins:
	//Wire.pins(12, 14); // SCL, SDA
	Wire.pins(4, 5);
	Wire.begin();

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk);
}

