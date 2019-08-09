#include <SmingCore.h>
#include <Network/SmtpClient.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

// Make sure to change those to your desired values
#define MAIL_FROM "admin@sming.com"
#define MAIL_TO "slav@attachix.com"
#define SMTP_USERNAME nullptr
#define SMTP_PASSWORD nullptr
#define SMTP_HOST "attachix.com"
#define SMTP_PORT 0 // Use default port
#define SMTP_USE_SSL false

SmtpClient client;

int onServerError(SmtpClient& client, int code, char* status)
{
	debugf("Status: %s", status);

	return 0; // return non-zero value to abort the connection
}

int onMailSent(SmtpClient& client, int code, char* status)
{
	// get the sent mail message
	MailMessage* mail = client.getCurrentMessage();

	// TODO: The status line contains the unique ID that was given to this email
	debugf("Mail sent. Status: %s", status);

	// And if there are no more pending emails then you can disconnect from the server
	if(!client.countPending()) {
		debugf("No more mails to send. Quitting...");
		client.quit();
	}

	return 0;
}

void onConnected(IpAddress ip, IpAddress mask, IpAddress gateway)
{
#ifdef ENABLE_SSL
	client.addSslOptions(SSL_SERVER_VERIFY_LATER);
#endif

	client.onServerError(onServerError);

	Url dsn(SMTP_USE_SSL ? URI_SCHEME_SMTP_SECURE : URI_SCHEME_SMTP, SMTP_USERNAME, SMTP_PASSWORD, SMTP_HOST,
			SMTP_PORT);
	debugf("Connecting to SMTP server using: %s", String(dsn).c_str());

	client.connect(dsn);

	MailMessage* mail = new MailMessage();
	mail->from = MAIL_FROM;
	mail->to = MAIL_TO;
	mail->subject = "Greetings from Sming";
	mail->setBody("Hello.\r\n.\r\n"
				  "This is test email from Sming <https://github.com/SmingHub/Sming>"
				  "It contains attachment, Ümlauts, кирилица + etc");

	FileStream* file = new FileStream("image.png");
	mail->addAttachment(file);

	client.onMessageSent(onMailSent);
	client.send(mail);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);
	Serial.println("Sming: SmtpClient example!");

	spiffs_mount();

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	WifiEvents.onStationGotIP(onConnected);
}
