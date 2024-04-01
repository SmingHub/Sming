#include <SmingCore.h>
#include <Network/SmtpClient.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
// Make sure to change those to your desired values
DEFINE_FSTR(MAIL_FROM, "admin@sming.com")
DEFINE_FSTR(MAIL_TO, "slav@attachix.com")
DEFINE_FSTR(SMTP_USERNAME, "")
DEFINE_FSTR(SMTP_PASSWORD, "")
DEFINE_FSTR(SMTP_HOST, "attachix.com")
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
	Serial << _F("Mail sent to '") << mail->to << _F("'. Status: ") << status << endl;

	// And if there are no more pending emails then you can disconnect from the server
	if(client.countPending() == 0) {
		Serial.println(_F("No more mails to send. Quitting..."));
		client.quit();
	}

	return 0;
}

void onConnected(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	client.setSslInitHandler([](Ssl::Session& session) { session.options.verifyLater = true; });

	client.onServerError(onServerError);

	Url dsn(SMTP_USE_SSL ? URI_SCHEME_SMTP_SECURE : URI_SCHEME_SMTP, SMTP_USERNAME, SMTP_PASSWORD, SMTP_HOST,
			SMTP_PORT);
	Serial << _F("Connecting to SMTP server using: ") << dsn << endl;

	client.connect(dsn);

	MailMessage* mail = new MailMessage();
	mail->from = MAIL_FROM;
	mail->to = MAIL_TO;
	mail->subject = "Greetings from Sming";
	String body = F("Hello.\r\n."
					"\r\n"
					"This is test email from Sming <https://github.com/SmingHub/Sming>\r\n"
					"It contains attachment, Ümlauts, кирилица + etc");
	// Note: Body can be quite large so use move semantics to avoid additional heap allocation
	mail->setBody(std::move(body));

	FileStream* file = new FileStream("image.png");
	mail->addAttachment(file);

	client.onMessageSent(onMailSent);
	client.send(mail);
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);
	Serial.println(_F("Sming: SmtpClient example!"));

	spiffs_mount();

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put your SSID and password here

	WifiEvents.onStationGotIP(onConnected);
}
