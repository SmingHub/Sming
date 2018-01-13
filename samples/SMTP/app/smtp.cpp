/**
* This sample shows how to send en email using SMTP. 
* Note that there can be issues with mail servers rejecting emails sent using this technique.
* A free account can be created using smtp2go
*/

#include <SmingCore/SmingCore.h>
#include "smtp.h"


//TODO get rid of this extern
extern smtpClient smtp;


#ifdef ENABLE_SSL
/* Debug SSL functions */
void displaySessionId(SSL *ssl)
{
	int i;
	const uint8_t *session_id = ssl_get_session_id(ssl);
	int sess_id_size = ssl_get_session_id_size(ssl);

	if (sess_id_size > 0)
	{
		debugf("-----BEGIN SSL SESSION PARAMETERS-----");
		for (i = 0; i < sess_id_size; i++)
		{
			m_printf("%02x", session_id[i]);
		}

		debugf("\n-----END SSL SESSION PARAMETERS-----");
	}
}

/**
* Display what cipher we are using
*/
void displayCipher(SSL *ssl)
{
	m_printf("CIPHER is ");
	switch (ssl_get_cipher_id(ssl))
	{
	case SSL_AES128_SHA:
		m_printf("AES128-SHA");
		break;

	case SSL_AES256_SHA:
		m_printf("AES256-SHA");
		break;

	case SSL_AES128_SHA256:
		m_printf("SSL_AES128_SHA256");
		break;

	case SSL_AES256_SHA256:
		m_printf("SSL_AES256_SHA256");
		break;

	default:
		m_printf("Unknown - %d", ssl_get_cipher_id(ssl));
		break;
	}

	m_printf("\n");
}
#endif

void onClientComplete(TcpClient& connection, bool success)
{
	debugf("onClientComplete");
	//client->close();
}



// ===========================================
// For receiving stuff back from SMTP server
// ===========================================
bool onReceive(TcpClient& tcpClient, char * data, int size) {
	debugf("Got data with size: %d", size);
	debugf("Free heap: %d", system_get_free_heap_size());
	if (size < 1) {
		return false;
	}
	for (int i = 0; i < size; i++)
	{
		char c = data[i];
		if (isPrintable(c))
		{
			Serial.print(c);
		}
	}
	Serial.println();


	if (smtp.showMeta)
	{
#ifdef ENABLE_SSL
		SSL* ssl = tcpClient.getSsl();
		if (ssl) {
			const char *common_name = ssl_get_cert_dn(ssl, SSL_X509_CERT_COMMON_NAME);
			if (common_name) {
				debugf("Common Name:\t\t\t%s\n", common_name);
			}
			displayCipher(ssl);
			displaySessionId(ssl);
		}
		debugf("end of meta...");
#endif
		smtp.showMeta = false;
	}

	//debugf("Ready to send back the data...");

	return true;//tcpClient.send(data, size);
}

smtpClient::smtpClient()
{
	client = nullptr;
	showMeta = true;
}
//TODO consider what the return value from this function should be...
byte smtpClient::sendEmail(
	const String&	emailFrom, 
	const String&	emailTo, 
	const String&	subject, 
	const String&	body)
{
	String s;
	const char * const CRLF = "\r\n";
	s = "EHLO ";  s += WifiStation.getIP().toString(); s += CRLF;
	Serial.print("Sending ");Serial.println(s);
	client->writeString(s);
	WDT.alive();

	Serial.println("Sending auth login");
	client->writeString("auth login\r\n");
	WDT.alive();
	
	Serial.println("Sending User");
	// Change to your base64, ASCII encoded user 
	// Use the base64decode() function
	client->writeString("YnJhbWJlbGw=\r\n"); // SMTP UserID brambell
	WDT.alive();

	Serial.println("Sending Password");
	// change to your base64, ASCII encoded password
	client->writeString("amVwMlpwRHI3aVhU\r\n");//  SMTP Passw
	WDT.alive();
		
	Serial.println("Sending From");   
	s = "MAIL From: "; s += emailFrom; s += CRLF;
	client->writeString(s); // not important 
	WDT.alive();

	
	Serial.println("Sending To");
	
	s = "RCPT To: "; s += emailTo; s += CRLF;
	client->writeString(s);
	WDT.alive();
	
	Serial.println("Sending DATA");
	client->writeString("DATA\r\n");
	WDT.alive();
	
	Serial.println("Sending email");   
	
	s = "To: "; s += emailTo; s +=" + \r\n"; 
	client->writeString(s); 

	s = "From: "; s += emailFrom; s += CRLF;
	client->writeString(s);//"From: al.paca@gmail.com\r\n");
	WDT.alive();

	s = "Subject: ";	s += subject;	s += CRLF;
	client->writeString(s);
	WDT.alive();

	//client->writeString("Power");
	//client->writeString(ESP.getVcc());
	//client->writeString(CRLF);

	client->writeString(body);
	WDT.alive();
	client->writeString(CRLF);
	WDT.alive();

	// Send a dot on a line of its own.
	s = "."; s += CRLF;
	client->writeString(s);
	WDT.alive();

	Serial.println("Sending QUIT");
	client->writeString("QUIT\r\n");
	WDT.alive();

	return 1;
}

void smtpClient::close()
{
	//client->close();
}
/*
byte eRcv()
{
	byte respCode;
	byte thisByte;
	int loopCount = 0;
	while (!client->available())
	{
		delay(1);
		loopCount++;     // if nothing received for 10 seconds, timeout
		if (loopCount > 10000) {
			client->stop();
			Serial.println("\r\nTimeout");
			return 0;
		}
	}

	respCode = client->peek();
	while (client->available())
	{
		thisByte = client->read();
		Serial.write(thisByte);
	}

	if (respCode >= '4')
	{
		//  efail();
		return 0;
	}
	return 1;
}
*/

void smtpClient::createClient(const String&  smtp_server, int smtp_port)
{
	client = new TcpClient(TcpClientCompleteDelegate(onClientComplete), TcpClientDataDelegate(onReceive));
#ifdef ENABLE_SSL	
	client->addSslOptions(SSL_SERVER_VERIFY_LATER);
	This code is untested . . . 
	bool erc = client->connect(smtp_server, smtp_port, true/*useSsl*/);
#else
	bool erc = client->connect(smtp_server.c_str(), smtp_port, false/*useSsl*/);
#endif	
	if (erc) {
		Serial.println("SMTP connected");
	}
	else {
		Serial.println("SMTP connection failed");
	}

}

/*
The following interaction is roughly what we might be expecting
..........
WiFi Connected
IP: 192.168.100.107
connected
220 mail.smtp2go.com ESMTP Exim 4.87 Sun, 07 Jan 2018 07:06:04 +0000
Sending EHLO 1801758912
250-mail.smtp2go.com Hello 1801758912 [121.218.193.62]
250-SIZE 52428800
250-8BITMIME
250-DSN
250-PIPELINING
250-AUTH CRAM-MD5 PLAIN LOGIN
250-STARTTLS
250-PRDR
250 HELP
Sending auth login
334 XXXXXXXXXXXX
Sending User
334 XXXXXXXXXXXX
Sending Password
235 Authentication succeeded
Sending From
250 OK
Sending To
250 Accepted <frank.esp8266@gmail.com>
Sending DATA
354 Enter message, ending with "." on a line by itself
Sending email
Voltage2766250 OK id=1eY51z-WfZb6W-GA
Sending QUIT
221 mail.smtp2go.com closing connection
disconnected

*/