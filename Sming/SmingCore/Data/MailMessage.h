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

#include "../../Wiring/WString.h"
#include "../../Wiring/WVector.h"
#include "../Network/WebConstants.h"

#include "Network/Http/HttpHeaders.h"
#include "Stream/MultipartStream.h"
#include "Stream/DataSourceStream.h"
#include "Stream/FileStream.h"

class SmtpClient;

class MailMessage
{
	friend class SmtpClient;

public:
	String to;
	String from;
	String subject;
	String cc;

	MailMessage& setHeader(const String& name, const String& value);

	HttpHeaders& getHeaders();

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
	MailMessage& addAttachment(FileStream* stream);

	/**
	 * @brief Adds attachment to the email
	 */
	MailMessage& addAttachment(ReadWriteStream* stream, MimeType mime, const String& filename = "");

	/**
	 * @brief Adds attachment to the email
	 */
	MailMessage& addAttachment(ReadWriteStream* stream, const String& mime, const String& filename = "");

	/**
	 * @brief Get the generated data stream
	 */
	ReadWriteStream* getData();

private:
	ReadWriteStream* stream = nullptr;
	HttpHeaders headers;
	Vector<HttpPartResult> attachments;
};

/** @} */
#endif /* _SMING_CORE_DATA_MESSAGE_H_ */
