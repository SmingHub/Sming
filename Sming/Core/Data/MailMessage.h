/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * MailMessage.h
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

/** @defgroup   smtpclient SMTP client
 *  @brief      Provides SMTP/S client
 *  @ingroup    tcpclient
 *  @{
 */

#pragma once

#include "WString.h"
#include "WVector.h"
#include "Network/WebConstants.h"
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

	/**
	 * @brief Set a header value
	 * @param name
	 * @param value
	 * @retval MailMessage&
	 */
	MailMessage& setHeader(const String& name, const String& value);

	/**
	 * @brief Get a reference to the current set of headers
	 * @retval HttpHeaders&
	 */
	HttpHeaders& getHeaders();

	/**
	 * @brief Sets the body of the email
	 * @param body
	 * @param mime
	 * @retval MailMessage&
	 */
	MailMessage& setBody(const String& body, MimeType mime = MIME_TEXT);

	/**
	 * @brief Sets the body of the email
	 * @param stream
	 * @param mime
	 * @retval MailMessage&
	 */
	MailMessage& setBody(IDataSourceStream* stream, MimeType mime = MIME_TEXT);

	/**
	 * @brief Adds attachment to the email
	 * @param stream
	 * @retval MailMessage&
	 */
	MailMessage& addAttachment(FileStream* stream);

	/**
	 * @brief Adds attachment to the email
	 * @param stream
	 * @param mime
	 * @param filename
	 * @retval MailMessage&
	 */
	MailMessage& addAttachment(IDataSourceStream* stream, MimeType mime, const String& filename = "");

	/**
	 * @brief Adds attachment to the email
	 * @param stream
	 * @param mime
	 * @param filename
	 * @retval MailMessage&
	 */
	MailMessage& addAttachment(IDataSourceStream* stream, const String& mime, const String& filename = "");

private:
	IDataSourceStream* stream = nullptr;
	HttpHeaders headers;
	Vector<HttpPartResult> attachments;
};

/** @} */
