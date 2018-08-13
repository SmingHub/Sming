/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

/** @defgroup   smtpclient SMTP client
 *  @brief      Provides SMTP/S client
 *  @ingroup    tcpclient
 *  @{
 */

#ifndef _SMING_CORE_DATA_MESSAGE_H_
#define _SMING_CORE_DATA_MESSAGE_H_

#include "WString.h"
#include "WVector.h"
#include "Network/WebConstants.h"
#include "Data/HttpHeaders.h"
#include "Stream/MultipartStream.h"

class TcpConnection;

class MailMessage
{
public:
	String to = nullptr;
	String from = nullptr;
	String subject = nullptr;
	String cc = nullptr;

	HttpHeaders headers;

	~MailMessage()
	{
		delete _bodyStream;
	}

	/**
	 * @brief Sets the body of the email
	 * @param String body
	 * @param MimeType mime
	 */
	MailMessage& setBody(const String& body, MimeType mime = MIME_TEXT);

	/**
	 * @brief Sets the body of the email
	 * @param Stream& stream
	 * @param MimeType mime
	 */
	MailMessage& setBody(ReadWriteStream* stream, MimeType mime = MIME_TEXT);

	/**
	 * @brief Adds attachment to the email
	 */
	MailMessage&  addAttachment(ReadWriteStream* stream);

	/**
	 * @brief Adds attachment to the email
	 */
	MailMessage&  addAttachment(ReadWriteStream* stream, MimeType mime, const String& filename = nullptr);

	/**
	 * @brief Adds attachment to the email
	 */
	MailMessage&  addAttachment(ReadWriteStream* stream, const String& mime, const String& filename = nullptr);

	/**
	 * Called by SmtpClient to prepare headers prior to sending
	 */
	HttpHeaders& prepareHeaders();

	/*
	 * Called by SmtpClient to get body stream, to whom ownership is passed
	 */
	IDataSourceStream* getBodyStream();

private:
	/**
	 * @brief Takes care to fetch the correct streams for a message
	 * @note The magic where all streams and attachments are packed together is happening here
	 */
	HttpPartResult multipartProducer();

private:
	IDataSourceStream* _bodyStream = nullptr;
	MimeType _bodyMime = MIME_TEXT;
	Vector<HttpPartResult> _attachments;
};

/** @} */
#endif /* _SMING_CORE_DATA_MESSAGE_H_ */
