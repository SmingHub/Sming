/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SmtpClient.cpp
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
#include <Data/WebHelpers/base64.h>
#include <Data/Stream/QuotedPrintableOutputStream.h>
#include <Data/Stream/Base64OutputStream.h>
#include <Data/HexString.h>
#include <Crypto/Md5.h>

#define ADVANCE                                                                                                        \
	{                                                                                                                  \
		buffer++;                                                                                                      \
		len--;                                                                                                         \
	}
#define ADVANCE_AND_BREAK                                                                                              \
	{                                                                                                                  \
		ADVANCE;                                                                                                       \
		break;                                                                                                         \
	}
#define ADVANCE_UNTIL_EOL                                                                                              \
	do {                                                                                                               \
		if(buffer[-1] == '\r' && buffer[0] == '\n') {                                                                  \
			ADVANCE_AND_BREAK;                                                                                         \
		}                                                                                                              \
		ADVANCE;                                                                                                       \
	} while(len > 0);

#define ADVANCE_UNTIL_EOL_OR_BREAK                                                                                     \
	{                                                                                                                  \
		ADVANCE_UNTIL_EOL;                                                                                             \
		if(buffer[-1] != '\n') {                                                                                       \
			break;                                                                                                     \
		}                                                                                                              \
	}

#define RETURN_ON_ERROR(SUCCESS_CODE)                                                                                  \
	if(codeValue != SUCCESS_CODE) {                                                                                    \
		memcpy(message, line, std::min(lineLength, SMTP_ERROR_LENGTH));                                                \
		message[SMTP_ERROR_LENGTH] = '\0';                                                                             \
		return 0;                                                                                                      \
	}

#define WAIT_FOR_STREAM(A)                                                                                             \
	if(A != nullptr && !A->isFinished()) {                                                                             \
		break;                                                                                                         \
	}

SmtpClient::~SmtpClient()
{
	delete outgoingMail;
	outgoingMail = nullptr;

	while(mailQ.count() != 0) {
		delete mailQ.dequeue();
	}
}

bool SmtpClient::connect(const Url& url)
{
	if(getConnectionState() != eTCS_Ready) {
		close();
	}

	bool isSecure = (url.Scheme == URI_SCHEME_SMTP_SECURE);
	this->url = url;
	return TcpClient::connect(url.Host, url.getPort(), isSecure);
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

bool SmtpClient::send(const String& from, const String& to, const String& subject, String&& body) noexcept
{
	MailMessage* mail = new MailMessage();

	mail->to = to;
	mail->from = from;
	mail->subject = subject;
	mail->setBody(std::move(body));

	return send(mail);
}

MailMessage* SmtpClient::getCurrentMessage()
{
	return outgoingMail;
}

bool SmtpClient::send(MailMessage* mail)
{
	if(!mailQ.enqueue(mail)) {
		// the mail queue is full
		delete mail;
		return false;
	}

	return true;
}

void SmtpClient::quit()
{
	sendString(F("QUIT\r\n"));
	state = eSMTP_Quitting;
}

// Protected Methods
void SmtpClient::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	switch(state) {
	case eSMTP_StartTLS: {
		sendString(F("STARTTLS\n\n"));
		state = eSMTP_Banner;
		break;
	}

	case eSMTP_SendAuth: {
		auto authPlain = [this]() {
			// base64('\0' + username + '\0' + password)
			String token = '\0' + url.User + '\0' + url.Password;
			String hash = base64_encode(token);
			sendString(F("AUTH PLAIN ") + hash + "\r\n");
			state = eSMTP_SendingAuth;
		};

		auto authCramMd5 = [this]() {
			// Slower cram-md5 authentication
			sendString(F("AUTH CRAM-MD5\r\n"));
			state = eSMTP_RequestingAuthChallenge;
		};

		DEFINE_FSTR_LOCAL(methodPlain, "PLAIN")
		DEFINE_FSTR_LOCAL(methodCramMd5, "CRAM-MD5")
		if(useSsl) {
			if(authMethods.contains(methodPlain)) {
				authPlain();
			} else if(authMethods.contains(methodCramMd5)) {
				authCramMd5();
			}
		} else if(authMethods.contains(methodCramMd5)) {
			authCramMd5();
		} else if(authMethods.contains(methodPlain)) {
			authPlain();
		}

		if(state == eSMTP_SendAuth) {
			state = eSMTP_Ready;
		}

		break;
	}

	case eSMTP_SendAuthResponse: {
		// Calculate the CRAM-MD5 response
		//     base64.b64encode("user " +hmac.new(password, base64.b64decode(challenge), hashlib.md5).hexdigest())

		auto digest = Crypto::HmacMd5(url.Password).calculate(authChallenge);
		String token = url.User + ' ' + Crypto::toString(digest);
		sendString(base64_encode(token) + "\r\n");
		state = eSMTP_SendingAuth;

		break;
	}

	case eSMTP_Ready: {
		delete outgoingMail;

		debugf("Queue size: %d", mailQ.count());

		outgoingMail = mailQ.dequeue();
		if(!outgoingMail) {
			break;
		}

		state = eSMTP_SendMail;
	}

	case eSMTP_SendMail: {
		sendString(F("MAIL FROM:") + outgoingMail->from + "\r\n");
		if(options & SMTP_OPT_PIPELINE) {
			sendString(F("RCPT TO:") + outgoingMail->to + "\r\n");
			sendString(F("DATA\r\n"));
		}

		state = eSMTP_SendingMail;
		break;
	}

	case eSMTP_SendRcpt: {
		sendString(F("RCPT TO:") + outgoingMail->to + "\r\n");
		state = eSMTP_SendingRcpt;
		break;
	}

	case eSMTP_SendData: {
		sendString(F("DATA\r\n"));
		state = eSMTP_SendingData;
		break;
	}

	case eSMTP_SendHeader: {
		WAIT_FOR_STREAM(stream);

		sendMailHeaders(outgoingMail);

		state = eSMTP_SendingHeaders;
	}

	case eSMTP_SendingHeaders: {
		WAIT_FOR_STREAM(stream);

		state = eSMTP_StartBody;
	}

	case eSMTP_StartBody: {
		sendMailBody(outgoingMail);
		state = eSMTP_SendingBody;
	}

	case eSMTP_SendingBody: {
		WAIT_FOR_STREAM(stream);

		// send the final dot
		state = eSMTP_Sent;
		delete stream;
		stream = nullptr;

		sendString(F("\r\n.\r\n"));
		break;
	}

	case eSMTP_Disconnect: {
		setTimeOut(1);
		return;
	}

	default:; // Do nothing

	} /* switch(state) */

	TcpClient::onReadyToSendData(sourceEvent);
}

MultipartStream::BodyPart SmtpClient::multipartProducer()
{
	MultipartStream::BodyPart result;

	if(!outgoingMail->attachments.isEmpty()) {
		result = outgoingMail->attachments[0];
		outgoingMail->attachments.remove(0);

		if(!result.headers->contains(HTTP_HEADER_CONTENT_TRANSFER_ENCODING)) {
			result.stream = new Base64OutputStream(result.stream);
			(*result.headers)[HTTP_HEADER_CONTENT_TRANSFER_ENCODING] = _F("base64");
		}
	}

	return result;
}

void SmtpClient::sendMailHeaders(MailMessage* mail)
{
	mail->getHeaders();

	if(!mail->headers.contains(HTTP_HEADER_CONTENT_TRANSFER_ENCODING)) {
		mail->headers[HTTP_HEADER_CONTENT_TRANSFER_ENCODING] = _F("quoted-printable");
		mail->stream = new QuotedPrintableOutputStream(mail->stream);
	}

	if(!mail->attachments.isEmpty()) {
		MultipartStream* mStream = new MultipartStream(MultipartStream::Producer(&SmtpClient::multipartProducer, this));
		MultipartStream::BodyPart text;
		text.headers = new HttpHeaders();
		(*text.headers)[HTTP_HEADER_CONTENT_TYPE] = mail->headers[HTTP_HEADER_CONTENT_TYPE];
		(*text.headers)[HTTP_HEADER_CONTENT_TRANSFER_ENCODING] = mail->headers[HTTP_HEADER_CONTENT_TRANSFER_ENCODING];
		text.stream = mail->stream;

		mail->attachments.insertElementAt(text, 0);

		mail->headers.remove(HTTP_HEADER_CONTENT_TRANSFER_ENCODING);
		mail->headers[HTTP_HEADER_CONTENT_TYPE] = F("multipart/mixed; boundary=") + mStream->getBoundary();
		mail->stream = mStream;
	}

	for(auto hdr : mail->headers) {
		sendString(hdr);
	}
	sendString("\r\n");
}

bool SmtpClient::sendMailBody(MailMessage* mail)
{
	if(mail->stream == nullptr) {
		return true;
	}

	delete stream;
	stream = mail->stream; // avoid intermediate buffers
	mail->stream = nullptr;

	return false;
}

err_t SmtpClient::onReceive(pbuf* buf)
{
	if(buf == nullptr) {
		return TcpClient::onReceive(buf);
	}

	pbuf* cur = buf;
	int parsedBytes = 0;
	while(cur != nullptr && cur->len > 0) {
		parsedBytes += smtpParse(static_cast<char*>(cur->payload), cur->len);
		cur = cur->next;
	}

	if(parsedBytes != buf->tot_len) {
		debug_e("[SMTP] Got error %s: %s", code, message);
		debug_hex(DBG, "SMTP", buf->payload, buf->len);

		if(!errorCallback || errorCallback(*this, codeValue, message) != 0) {
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
	while(len) {
		char currentByte = *buffer;
		// parse the code...
		if(codeLength < 3) {
			code[codeLength++] = currentByte;
			ADVANCE;
			continue;
		}

		if(codeLength == 3) {
			code[codeLength] = '\0';
			if(currentByte != ' ' && currentByte != '-') {
				// the code must be followed by space or minus
				return 0;
			}

			codeValue = strtol(code, nullptr, 10);
			isLastLine = (currentByte == ' ');
			codeLength++;
			ADVANCE;
		}

		char* line = buffer;
		ADVANCE_UNTIL_EOL_OR_BREAK;
		codeLength = 0;
		int lineLength = (buffer - line) - 2;

		switch(state) {
		case eSMTP_Banner: {
			RETURN_ON_ERROR(SMTP_CODE_SERVICE_READY);

			if(!useSsl && (options & SMTP_OPT_STARTTLS)) {
				useSsl = true;
				TcpConnection::internalOnConnected(ERR_OK);
			}

			sendString(F("EHLO ") + url.Host + "\r\n");
			state = eSMTP_Hello;

			break;
		}

		case eSMTP_Hello: {
			RETURN_ON_ERROR(SMTP_CODE_REQUEST_OK);

			if(strncmp(line, _F("PIPELINING"), lineLength) == 0) {
				// PIPELINING (see: https://tools.ietf.org/html/rfc2920)
				options |= SMTP_OPT_PIPELINE;
			} else if(strncmp(line, _F("STARTTLS"), lineLength) == 0) {
				// STARTTLS (see: https://www.ietf.org/rfc/rfc3207.txt)
				options |= SMTP_OPT_STARTTLS;
			} else if(strncmp(line, _F("AUTH "), 5) == 0) {
				// Process authentication methods
				// Ex: 250-AUTH CRAM-MD5 PLAIN LOGIN
				// See: https://tools.ietf.org/html/rfc4954
				String text(line + 5, lineLength - 5);
				splitString(text, ' ', authMethods);
			}

			if(isLastLine) {
				state = eSMTP_Ready;
				if(!useSsl && (options & SMTP_OPT_STARTTLS)) {
					state = eSMTP_StartTLS;
				} else if(url.User && authMethods.count()) {
					state = eSMTP_SendAuth;
				}
			}

			break;
		}

		case eSMTP_RequestingAuthChallenge: {
			RETURN_ON_ERROR(SMTP_CODE_AUTH_CHALLENGE);
			authChallenge = base64_decode(line, lineLength);
			state = eSMTP_SendAuthResponse;

			break;
		}

		case eSMTP_SendingAuth: {
			RETURN_ON_ERROR(SMTP_CODE_AUTH_OK);

			authMethods.clear();

			state = eSMTP_Ready;

			break;
		}

		case eSMTP_SendingMail: {
			RETURN_ON_ERROR(SMTP_CODE_REQUEST_OK);

			state = ((options & SMTP_OPT_PIPELINE) ? eSMTP_SendingRcpt : eSMTP_SendRcpt);

			break;
		}

		case eSMTP_SendingRcpt: {
			RETURN_ON_ERROR(SMTP_CODE_REQUEST_OK);

			state = ((options & SMTP_OPT_PIPELINE) ? eSMTP_SendingData : eSMTP_SendData);

			break;
		}
		case eSMTP_SendingData: {
			RETURN_ON_ERROR(SMTP_CODE_START_DATA);

			state = eSMTP_SendHeader;

			break;
		}
		case eSMTP_Sent: {
			RETURN_ON_ERROR(SMTP_CODE_REQUEST_OK);

			state = eSMTP_Ready;

			if(messageSentCallback) {
				messageSentCallback(*this, codeValue, message);
			}
			delete outgoingMail;
			outgoingMail = nullptr;

			break;
		}

		case eSMTP_Quitting: {
			RETURN_ON_ERROR(SMTP_CODE_BYE);
			state = eSMTP_Disconnect;

			break;
		}

		default:
			memcpy(message, line, std::min(lineLength, SMTP_ERROR_LENGTH));

		} /* switch(state) */
	}

	return (buffer - start);
}
