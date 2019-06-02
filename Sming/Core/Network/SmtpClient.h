/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SmtpClient.h - Asynchronous SmtpClient that supports the following features:
 *
 * - extended HELO command set
 * - support for PIPELINING
 * - support for STARTTLS (if the directive ENABLE_SSL=1 is set)
 * - support for smtp connection over SSL (if the directive ENABLE_SSL=1 is set)
 * - support for PLAIN and CRAM-MD5 authentication
 * - support for multiple attachments
 * - Support for base64 and quoted-printable transfer encoding
 *
 * Author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

/** @defgroup   smtpclient SMTP client
 *  @brief      Provides SMTP/S client
 *  @ingroup    tcpclient
 *  @{
 */

#include "TcpClient.h"
#include "Data/MailMessage.h"
#include "Url.h"
#include "WString.h"
#include "WVector.h"
#include "Data/Stream/DataSourceStream.h"
#include "WebConstants.h"
#include "Data/ObjectQueue.h"

#include <functional>

/* Maximum waiting emails in the mail queue */
#define SMTP_QUEUE_SIZE 5

/* Buffer size used to read the error messages */
#define SMTP_ERROR_LENGTH 40

/**
 * SMTP response codes
 */
#define SMTP_CODE_SERVICE_READY 220
#define SMTP_CODE_BYE 221
#define SMTP_CODE_AUTH_OK 235
#define SMTP_CODE_REQUEST_OK 250
#define SMTP_CODE_AUTH_CHALLENGE 334
#define SMTP_CODE_START_DATA 354

#define SMTP_OPT_PIPELINE bit(0)
#define SMTP_OPT_STARTTLS bit(1)
#define SMTP_OPT_AUTH_PLAIN bit(2)
#define SMTP_OPT_AUTH_LOGIN bit(3)
#define SMTP_OPT_AUTH_CRAM_MD5 bit(4)

enum SmtpState {
	eSMTP_Banner = 0,
	eSMTP_Hello,
	eSMTP_StartTLS,
	eSMTP_SendAuth,
	eSMTP_SendingAuthLogin,
	eSMTP_RequestingAuthChallenge,
	eSMTP_SendAuthResponse,
	eSMTP_SendingAuth,
	eSMTP_Ready,
	eSMTP_SendMail,
	eSMTP_SendingMail,
	eSMTP_SendRcpt,
	eSMTP_SendingRcpt,
	eSMTP_SendData,
	eSMTP_SendingData,
	eSMTP_SendHeader,
	eSMTP_SendingHeaders,
	eSMTP_StartBody,
	eSMTP_SendingBody,
	eSMTP_Sent,
	eSMTP_Quitting,
	eSMTP_Disconnect
};

class SmtpClient;

typedef std::function<int(SmtpClient& client, int code, char* status)> SmtpClientCallback;

class SmtpClient : protected TcpClient
{
public:
	SmtpClient(bool autoDestroy = false);
	~SmtpClient();

	/**
	 * @brief Connects to remote URL
	 * @param URL - provides the protocol, remote server, port and user credentials
	 * 				allowed protocols:
	 * 					- smtp  - clear text SMTP
	 * 					- smtps - SMTP over SSL connection
	 */
	bool connect(const Url& url);

	/**
	 * @brief Queues a single message before it is sent later to the SMTP server
	 *
	 * @param String& from
	 * @param String& to
	 * @param String& subject
	 * @param String& body the body in plain text format
	 *
	 * @return true when the message was queued successfully, false otherwise
	 */
	bool send(const String& from, const String& to, const String& subject, const String& body);

	/**
	 * @brief Powerful method to queues a single message before it is sent later to the SMTP server
	 * @param MailMessage* message
	 *
	 * @return true when the message was queued successfully, false otherwise
	 */
	bool send(MailMessage* message);

	/**
	 * @brief Gets the current message
	 *
	 * @return MailMessage* message - the message, or NULL if none is scheduled
	 */
	MailMessage* getCurrentMessage();

	size_t countPending()
	{
		return mailQ.count();
	}

	/**
	 * @brief Sends a quit command to the server and closes the TCP conneciton
	 */
	void quit();

	/**
	 * @brief Returns the current state of the SmtpClient.
	 */
	SmtpState getState()
	{
		return state;
	}

	/**
	 * @brief Callback that will be called every time a message is sent successfully
	 * @param SmtpClientCallback callback
	 */
	void onMessageSent(SmtpClientCallback callback)
	{
		messageSentCallback = callback;
	}

	/**
	 * @brief Callback that will be called every an error occurs
	 * @param SmtpClientCallback callback
	 */
	void onServerError(SmtpClientCallback callback)
	{
		errorCallback = callback;
	}

	using TcpClient::setTimeOut;

#ifdef ENABLE_SSL
	using TcpClient::addSslOptions;
	using TcpClient::addSslValidator;
	using TcpClient::freeSslKeyCert;
	using TcpClient::getSsl;
	using TcpClient::pinCertificate;
	using TcpClient::setSslKeyCert;
#endif

protected:
	err_t onReceive(pbuf* buf) override;
	void onReadyToSendData(TcpConnectionEvent sourceEvent) override;

	void sendMailHeaders(MailMessage* mail);
	bool sendMailBody(MailMessage* mail);

private:
	Url url;
	Vector<String> authMethods;
	ObjectQueue<MailMessage, SMTP_QUEUE_SIZE> mailQ;
	char code[4] = {0};
	int codeValue = 0;
	String authChallenge;
	char message[SMTP_ERROR_LENGTH + 1] = {0};
	bool isLastLine = false;
	uint8_t codeLength = 0;
	int options = 0;
	MailMessage* outgoingMail = nullptr;
	SmtpState state = eSMTP_Banner;

	SmtpClientCallback errorCallback = nullptr;
	SmtpClientCallback messageSentCallback = nullptr;

private:
	/**
	 * @brief Simple and naive SMTP parser with a state machine
	 */
	int smtpParse(char* data, size_t len);

	/**
	 * @brief Takes care to fetch the correct streams for a message
	 * @note The magic where all streams and attachments are packed together is happening here
	 */
	HttpPartResult multipartProducer();
};
