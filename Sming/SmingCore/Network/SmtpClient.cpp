/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SmtpClient
 *
 * Author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

/** @defgroup   smtpclient SMTP client
 *  @brief      Provides SMTP/S client
 *  @ingroup    tcpclient
 *  @{
 */

#include "SmtpClient.h"
#include "../Services/WebHelpers/base64.h"
#include "Data/Stream/QuotedPrintableOutputStream.h"
#include "Data/Stream/Base64OutputStream.h"

#if !defined(ENABLE_SSL) || ENABLE_SSL == 0
// if our SSL is not used then we try to use the one coming from the SDK
#define MD5_SIZE 16
extern "C" {
void ssl_hmac_md5(const uint8_t* msg, int length, const uint8_t* key, int key_len, uint8_t* digest);
}
#define hmac_md5(A, B, C, D, E) ssl_hmac_md5(A, B, C, D, E)
#endif

#define ADVANCE                                                                                                        \
	{                                                                                                                  \
		buffer++;                                                                                                      \
		len--;                                                                                                         \
	}

#define ADVANCE_UNTIL_EOL                                                                                              \
	do {                                                                                                               \
		if (*(buffer - 1) == '\r' && *buffer == '\n') {                                                                \
			ADVANCE;                                                                                                   \
			break;                                                                                                     \
		}                                                                                                              \
		ADVANCE;                                                                                                       \
	} while (len > 0)

#define ADVANCE_UNTIL_EOL_OR_BREAK                                                                                     \
	{                                                                                                                  \
		ADVANCE_UNTIL_EOL;                                                                                             \
		if (buffer[-1] != '\n')                                                                                        \
			break;                                                                                                     \
	}

#define RETURN_ON_ERROR(SUCCESS_CODE)                                                                                  \
	if (_codeValue != SUCCESS_CODE) {                                                                                  \
		memcpy(_message, line, std::min(lineLength, SMTP_ERROR_LENGTH));                                               \
		_message[SMTP_ERROR_LENGTH] = '\0';                                                                            \
		return 0;                                                                                                      \
	}

SmtpClient::SmtpClient(bool autoDestroy /* =false */) : TcpClient(autoDestroy)
{}

SmtpClient::~SmtpClient()
{
	delete _outgoingMail;

	while (_mailQ.count())
		delete _mailQ.dequeue();
}

bool SmtpClient::connect(const URL& to_url)
{
	if (getConnectionState() != eTCS_Ready)
		close();

	_url = to_url;

	bool isSSL = _url.protocol() == SMTP_OVER_SSL_PROTOCOL;

	if (_url.port() == 0)
		_url.setPort(isSSL ? 465 : 25);

	return TcpClient::connect(_url.host(), _url.port(), isSSL);
}

bool SmtpClient::send(const String& from, const String& to, const String& subject, const String& body)
{
	MailMessage* mail = new MailMessage();

	mail->to = to;
	mail->from = from;
	mail->subject = subject;
	mail->setBody(body);

	return send(mail);
}

MailMessage* SmtpClient::getCurrentMessage()
{
	return _outgoingMail;
}

bool SmtpClient::send(MailMessage* mail)
{
	if (!_mailQ.enqueue(mail)) {
		// the mail queue is full
		delete mail;
		return false;
	}

	return true;
}

void SmtpClient::quit()
{
	sendString(_F("QUIT\r\n"));
	_state = eSMTP_Quitting;
}

// Protected Methods
void SmtpClient::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	switch (_state) {
	case eSMTP_StartTLS:
		sendString(_F("STARTTLS\n\n"));
		_state = eSMTP_Banner;
		break;

	case eSMTP_SendAuth:
		if (_authMethods.count()) {
			// TODO: Simplify the code in that block...
			Vector<String> preferredOrder;
			if (_useSsl) {
				preferredOrder.addElement(F("PLAIN"));
				preferredOrder.addElement(F("CRAM-MD5"));
			}
			else {
				preferredOrder.addElement(F("CRAM-MD5"));
				preferredOrder.addElement(F("PLAIN"));
			}

			for (unsigned i = 0; i < preferredOrder.count(); i++) {
				if (!_authMethods.contains(preferredOrder[i]))
					continue;

				if (preferredOrder[i] == _F("PLAIN")) {
					// base64('\0' + username + '\0' + password)
					String token = '\0' + _url.user() + '\0' + _url.password();
					String hash = base64_encode(token);
					sendString(F("AUTH PLAIN ") + hash + "\r\n");
					_state = eSMTP_SendingAuth;
					break;
				}

				if (preferredOrder[i] == F("CRAM-MD5")) {
					// otherwise we can try the slow cram-md5 authentication...
					sendString(F("AUTH CRAM-MD5\r\n"));
					_state = eSMTP_RequestingAuthChallenge;
					break;
				}
			}
		} /* authMethods.count */

		if (_state == eSMTP_SendAuth)
			_state = eSMTP_Ready;

		break;

	case eSMTP_SendAuthResponse: {
		// Calculate the CRAM-MD5 response
		//     base64.b64encode("user " +hmac.new(password, base64.b64decode(challenge), hashlib.md5).hexdigest())
		uint8_t digest[MD5_SIZE];
		hmac_md5((const uint8_t*)_authChallenge.c_str(), _authChallenge.length(),
				 (const uint8_t*)_url.password().c_str(), _url.password().length(), digest);

		String token = _url.user() + " " + toHexString(digest, MD5_SIZE);
		sendString(base64_encode(token) + "\r\n");
		_state = eSMTP_SendingAuth;

		break;
	}

	case eSMTP_Ready:
		delete _outgoingMail;
		debugf("Queue size: %d", _mailQ.count());
		_outgoingMail = _mailQ.dequeue();
		if (!_outgoingMail)
			break;

		_state = eSMTP_SendMail;
		// fall through

	case eSMTP_SendMail:
		sendString(_F("MAIL FROM:") + _outgoingMail->from + "\r\n");
		if (_options & SMTP_OPT_PIPELINE) {
			sendString(_F("RCPT TO:") + _outgoingMail->to + "\r\n");
			sendString(_F("DATA\r\n"));
		}
		_state = eSMTP_SendingMail;
		break;

	case eSMTP_SendRcpt:
		sendString(_F("RCPT TO:") + _outgoingMail->to + "\r\n");
		_state = eSMTP_SendingRcpt;
		break;

	case eSMTP_SendData:
		sendString(_F("DATA\r\n"));
		_state = eSMTP_SendingData;
		break;

	case eSMTP_SendHeader: {
		if (isSending())
			break;

		HttpHeaders& headers = _outgoingMail->prepareHeaders();
		for (unsigned i = 0; i < headers.count(); i++)
			writeString(headers[i]);
		writeString("\r\n");

		_state = eSMTP_SendingHeaders;
		// fall through
	}

	case eSMTP_SendingHeaders:
		if (isSending())
			break;
		_state = eSMTP_StartBody;
		// fall through

	case eSMTP_StartBody:
		TcpClient::send(_outgoingMail->getBodyStream());
		_state = eSMTP_SendingBody;
		// Fall through

	case eSMTP_SendingBody:
		if (isSending())
			break;

		// send the final dot
		_state = eSMTP_Sent;

		sendString(_F("\r\n.\r\n"));
		break;

	case eSMTP_Disconnect:
		close();
		return;

	case eSMTP_Banner:
	case eSMTP_Hello:
	case eSMTP_SendingAuthLogin:
	case eSMTP_RequestingAuthChallenge:
	case eSMTP_SendingAuth:
	case eSMTP_SendingMail:
	case eSMTP_SendingRcpt:
	case eSMTP_SendingData:
	case eSMTP_Sent:
	case eSMTP_Quitting:;

	} /* switch(state) */

	TcpClient::onReadyToSendData(sourceEvent);
}

err_t SmtpClient::onReceive(pbuf* buf)
{
	if (!buf)
		return TcpClient::onReceive(buf);

	int parsedBytes = 0;
	for (pbuf* cur = buf; cur && cur->len > 0; cur = cur->next)
		parsedBytes += smtpParse((char*)cur->payload, cur->len);

	if (parsedBytes != buf->tot_len) {
		debug_e("Got error: %s:%s", _code, _message);

		if (!_errorCallback || _errorCallback(*this, _codeValue, _message) != 0) {
			// abort the connection if we cannot handle it.
			TcpClient::onReceive(nullptr);

			return ERR_ABRT;
		}
	}

	TcpClient::onReceive(buf);

	return ERR_OK;
}

int SmtpClient::smtpParse(char* buffer, size_t len)
{
	char* start = buffer;
	while (len) {
		char currentByte = *buffer;
		// parse the code...
		if (_codeLength < 3) {
			_code[_codeLength++] = currentByte;
			ADVANCE;
			continue;
		}
		else if (_codeLength == 3) {
			_code[_codeLength] = '\0';
			if (currentByte != ' ' && currentByte != '-') {
				// the code must be followed by space or minus
				return 0;
			}

			char* tmp;
			_codeValue = strtol(_code, &tmp, 10);
			_isLastLine = (currentByte == ' ');
			_codeLength++;
			ADVANCE;
		}

		char* line = buffer;
		ADVANCE_UNTIL_EOL_OR_BREAK;
		_codeLength = 0;
		int lineLength = (buffer - line) - 2;

		switch (_state) {
		case eSMTP_Banner: {
			RETURN_ON_ERROR(SMTP_CODE_SERVICE_READY);

			if (!_useSsl && (_options & SMTP_OPT_STARTTLS)) {
				_useSsl = true;
				TcpConnection::_onConnected(_tcp, ERR_OK);
			}

			sendString(F("EHLO ") + _url.host() + "\r\n");
			_state = eSMTP_Hello;

			break;
		}

		case eSMTP_Hello: {
			RETURN_ON_ERROR(SMTP_CODE_REQUEST_OK);

			if (strncmp(line, _F("PIPELINING"), lineLength) == 0) {
				// PIPELINING (see: https://tools.ietf.org/html/rfc2920)
				_options |= SMTP_OPT_PIPELINE;
			}
			else if (strncmp(line, _F("STARTTLS"), lineLength) == 0) {
				// STARTTLS (see: https://www.ietf.org/rfc/rfc3207.txt)
				_options |= SMTP_OPT_STARTTLS;
			}
			else if (strncmp(line, _F("AUTH "), 5) == 0) {
				// Process authentication methods
				// Ex: 250-AUTH CRAM-MD5 PLAIN LOGIN
				// See: https://tools.ietf.org/html/rfc4954
				int offset = 0;
				int pos = -1;

				String text(line + 5, lineLength - 5);
				text.split(' ', _authMethods);
			}

			if (_isLastLine) {
				_state = eSMTP_Ready;
#ifdef ENABLE_SSL
				if (!_useSsl && (_options & SMTP_OPT_STARTTLS)) {
					_state = eSMTP_StartTLS;
				}
				else
#endif
					if (_url.user() && _authMethods.count()) {
					_state = eSMTP_SendAuth;
				}
			}

			break;
		}

		case eSMTP_RequestingAuthChallenge: {
			RETURN_ON_ERROR(SMTP_CODE_AUTH_CHALLENGE);
			_authChallenge = base64_decode(line, lineLength);
			_state = eSMTP_SendAuthResponse;

			break;
		}

		case eSMTP_SendingAuth: {
			RETURN_ON_ERROR(SMTP_CODE_AUTH_OK);

			_authMethods.clear();

			_state = eSMTP_Ready;

			break;
		}

		case eSMTP_SendingMail: {
			RETURN_ON_ERROR(SMTP_CODE_REQUEST_OK);

			_state = ((_options & SMTP_OPT_PIPELINE) ? eSMTP_SendingRcpt : eSMTP_SendRcpt);

			break;
		}

		case eSMTP_SendingRcpt: {
			RETURN_ON_ERROR(SMTP_CODE_REQUEST_OK);

			_state = ((_options & SMTP_OPT_PIPELINE) ? eSMTP_SendingData : eSMTP_SendData);

			break;
		}

		case eSMTP_SendingData: {
			RETURN_ON_ERROR(SMTP_CODE_START_DATA);

			_state = eSMTP_SendHeader;

			break;
		}

		case eSMTP_Sent: {
			RETURN_ON_ERROR(SMTP_CODE_REQUEST_OK);

			_state = eSMTP_Ready;

			if (_messageSentCallback)
				_messageSentCallback(*this, _codeValue, _message);

			delete _outgoingMail;
			_outgoingMail = nullptr;

			break;
		}

		case eSMTP_Quitting: {
			RETURN_ON_ERROR(SMTP_CODE_BYE);
			close();
			_state = eSMTP_Disconnect;

			break;
		}

		default:
			memcpy(_message, line, std::min(lineLength, SMTP_ERROR_LENGTH));

		} /* switch(state) */
	}

	return (buffer - start);
}
