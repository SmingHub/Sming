#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "SmingCore/Network/SmtpClient.h"
#include <SmingCore/Network/SmtpClient.h>
#include "LedBlinker.h"


/*
* This sample shows how to send en email using SMTP.
* Note that there can be issues with mail servers rejecting emails sent using SMTP.
* A free account can be created using smtp2go
*/

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	// Put your SSID and Password here, also the email address you want to use
	#define WIFI_SSID		"SSID" 
	#define WIFI_PWD		"PWD"
	#define SMTP_SERVER		"mail.smtp2go.com"
	#define EMAIL_TO		"ele.fant@gmail.com"
	#define SMTP_USER		"SMTPUSER"
	#define SMTP_PASSWORD	"SMTPPASSWORD"
	#define SMTP_PORT		2525
#endif

SmtpClient smtp;


void serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount);

// ===========================================
// Heartbeat LED

LedBlinker	blink;


// ==============================================
// Will be called when WiFi station has connected
// After this point we can send an email...

void connectOk(IPAddress ip, IPAddress mask, IPAddress gateway)
{
	debugf("connectOk: I'm CONNECTED");
	Serial.print("connectOk: IP: ");
	Serial.print(ip.toString());
	Serial.print("   Gateway: ");
	Serial.println(gateway.toString());
	Serial.println();

	// We are now in a state where we are ready to send an email
	Serial.println("Press e to send an email");
}

// ==============================================
// Will be called when WiFi station was disconnected

void connectFail(String ssid, uint8_t ssidLength, uint8_t *bssid, uint8_t reason)
{
	// The different reason codes can be found in user_interface.h. in your SDK.
	debugf("connectFail: Disconnected from %s. Reason: %d", ssid.c_str(), reason);
}

// Will be called when WiFi hardware and software initialization was finished
// And system initialization was completed
void ready()
{
	debugf("READY!");
}

void init()
{
	Serial.begin(74880);//SERIAL_BAUD_RATE);
	
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println("Sming. Let's send an email notification!");
	Serial.setCallback(serialCallBack);

	blink.setup(2);
	blink.speed(LedBlinker::Fast);
	
	// Set system ready callback method
	System.onReady(ready);
	WifiAccessPoint.enable(false);
	
	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here
	
	// Set callback that should be triggered when we have assigned IP
	WifiEvents.onStationGotIP(connectOk);
	
	// Set callback that should be triggered if we are disconnected or connection attempt failed
	WifiEvents.onStationDisconnect(connectFail);
	
	blink.speed(LedBlinker::Slow);
}

void serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount)
{
	if (arrivedChar == '\n') {
		char str[availableCharsCount];
		for (int i = 0; i < availableCharsCount; i++) 
		{
			str[i] = stream.read();
			if (str[i] == '\r' || str[i] == '\n') {
				str[i] = '\0';
			}
		}
		if (!strcmp(str, "ip")) 
		{
			Serial.printf("ip: %s mac: %s\r\n",
				WifiStation.getIP().toString().c_str(),
				WifiStation.getMAC().c_str());
		}
		else if (!strcmp(str, "e")) 
		{

			String	smtp_server(SMTP_SERVER);
			String	smtp_user(SMTP_USER);
			String	smtp_password(SMTP_PASSWORD);

			smtp.setCredentials(smtp_server, smtp_user, smtp_password, SMTP_PORT);
			
			String subject = "Demonstrate the use of the TcpClient class and SMTP.";
			String body = "Test email.  Pop the champagne.\n";
			body += String("SDK:     ") + system_get_sdk_version() + "\n";
			body += String("CPU Freq:") + system_get_cpu_freq() + "\n";
			body += String("Chip ID: ") + String(system_get_chip_id(), HEX) + "\n";
			body += String("Flash ID:") + String(spi_flash_get_id(),HEX) + "\n";			
			body += String("Free Heap: ") + system_get_free_heap_size() + "\n";
			String emailFrom = "al.paca@gmail.com";
			String emailTo = EMAIL_TO;
						
			smtp.sendEmail(emailFrom, emailTo, subject, body);
		}
		else 
		{
			Serial.println();
			Serial.println("available commands:");
			Serial.println("  help - display this message");
			Serial.println("  ip - show current ip address");
			Serial.println("  e - send an email");
			Serial.println();
		}
	}
}
