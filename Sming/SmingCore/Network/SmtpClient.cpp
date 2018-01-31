#include "TcpClient.h"
#include "../../Platform/Station.h"
#include "../../Services/WebHelpers/base64.h"
#include "SmtpClient.h"


const char * const CRLF = "\r\n";
String makeBase64(const String& s);


void onClientComplete(TcpClient& connection, bool success)
{
	debugf("onClientComplete, %s", success? "true":"false");
}


bool SmtpClientReceive(TcpClient& tcpClient, char * data, int size) {
	debugf("SmtpClientReceive: Got data [%s] with size: %d", data, size);
	debugf("Free heap: %d", system_get_free_heap_size());
	if (size < 1) {
		return true;
	}
	return true;
}

void SmtpClient::setCredentials(
	const String&	smtp_server,
	const String&	pUser,
	const String&	pPassword,
	int				smtp_port)

{
	smtpUser = makeBase64(pUser);
	smtpPassword = makeBase64(pPassword);
	smtpServer = smtp_server;
	smtpPort = smtp_port;
}

// ===========================================
// Connect, and remember data to send later inside onReadyToSend
// ===========================================
void SmtpClient::sendEmail(
	const String&	emailFrom,
	const String&	emailTo,
	const String&	subject,
	const String&	body)
{
	debugf("SmtpClient::sendEmail");
#ifdef ENABLE_SSL	
	addSslOptions(SSL_SERVER_VERIFY_LATER);
	//This code is untested . . . 
	bool erc = TcpClient::connect(smtpServer, smtpPort, true/*useSsl*/);
#else
	bool erc = TcpClient::connect(smtpServer.c_str(), smtpPort, false/*useSsl*/);
#endif	
	if (erc) {
		debugf("SMTP connected\n");
	}
	else {
		debugf("SMTP connection failed\n");
	}

	String s;
	s = "EHLO ";  s += WifiStation.getIP().toString(); s += CRLF;
	msg[0] = s;
	msg[1] = "auth login\r\n";
	msg[2] = smtpUser + CRLF; // SMTP UserID 
	msg[3] = smtpPassword + CRLF;//  SMTP Password
	s = "MAIL From: "; s += emailFrom; s += CRLF;
	msg[4] = s; // not important 
	s = "RCPT To: "; s += emailTo; s += CRLF;
	msg[5] = s;
	msg[6] = "DATA\r\n";
	s = "To: "; s += emailTo; s +=" + \r\n"; 
	msg[7] = s;
	s = "From: "; s += emailFrom; s += CRLF;
	msg[8] = s;
	s = "Subject: ";	s += subject;	s += CRLF;
	msg[9] = s;
	msg[10] = body;
	msg[11] = CRLF;
	
	// Send a dot on a line of its own.
	s = "."; s += CRLF;
	msg[12] = s;
	msg[13] = "QUIT\r\n";

	// The next thing to happen after this function finishes is (if we are successful at signing on), 
	// is the function "onReadyToSendData"
}

err_t SmtpClient::onConnected(err_t err)
{
	debugf("SmtpClient::onConnected %d", (int)err);
	return TcpClient::onConnected(err);
}
err_t SmtpClient::onReceive(pbuf *buf)
{
	// Remove next line maybe?
	debugf("SmtpClient::onReceive [%s]", buf);
	return TcpClient::onReceive(buf);
}
err_t SmtpClient::onSent(uint16_t len)
{
	debugf("SmtpClient::onSent %d", len);
	return TcpClient::onSent(len);
}
void SmtpClient::onError(err_t err)
{
	debugf("SmtpClient::onError %d", (int)err);
	return TcpClient::onError(err);
}


void SmtpClient::onFinished(TcpClientState finishState)
{
	debugf("SmtpClient::onFinished %d", (int)finishState);
	uint8_t failed = (finishState == eTCS_Failed);
	if (failed)
	{
		debugf("Tcp Client failure...");
	}
	TcpClient::onFinished(finishState);
}

void SmtpClient::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	debugf("SmtpClient::onReadyToSendData");
	if (msg[0].length() > 0)
	{
		for (auto& s : msg)
		{
			//debugf("Sending: %s", s.c_str());
			bool forceCloseAfterSent = false;
			sendString(s, forceCloseAfterSent);
			//s = "";
		}
	}
	TcpClient::onReadyToSendData(sourceEvent);
}


// Base64 encoded strings are four thirds longer than the original 
// plus padding to get to an even multiple of three.
// Len    encoded length
//   0      0
//   1      4
//   2      4
//   3      4
//   4      8
//   5      8
//   6      8
int lengthBase64(int len)
{
	int missing = 0;
	size_t ret = len;
	size_t remainder = (ret % 3);
	if (remainder)
	{
		missing = 3 - remainder;
	}
	ret += missing;
	// Expand the return string size to a multiple of 4
	ret = 4 * ret / 3;
	return ret;
}



// Not used
String XXmakeBase64(const String& s)
{
	String ret;
	const int len = 64;
	//debugf("makeBase64 %s %d %d", s.c_str(), s.length(), len);
	char b64[len+1];
	memset(b64, 0, len + 1);
	base64_encode(s.length(), (const unsigned char *)s.c_str(), len, b64);
	ret = b64;
	return ret;
}


String makeBase64(const String& s)
{
	int len = lengthBase64(s.length());
	char * b64 = new char[len + 1];
	memset(b64, 0, len + 1);
	base64_encode(s.length(), (const unsigned char *)s.c_str(), len, b64);
	String ret = b64;
	delete[] b64;
	return ret;
}

SmtpClient::SmtpClient(bool autoDestroy)
	:TcpClient(autoDestroy),
	smtpUser(),
	smtpPassword()
{
	//setReceiveDelegate(SmtpClientReceive);
	setCompleteDelegate(onClientComplete);
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
334 XXXXXXXXXXXX        base64 username
Sending User
334 XXXXXXXXXXXX		base64 password
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