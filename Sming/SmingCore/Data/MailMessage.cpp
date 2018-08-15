/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "MailMessage.h"
#include "Stream/MemoryDataStream.h"
#include "Stream/QuotedPrintableOutputStream.h"
#include "Stream/Base64OutputStream.h"


MailMessage& MailMessage::setBody(const String& body, MimeType mime /* = MIME_TEXT */)
{
	MemoryDataStream* memory = new MemoryDataStream();
	size_t written = memory->print(body);
	if (written < body.length())
		debug_e("MailMessage::setBody: Unable to store the complete body");

	return setBody(memory, mime);
}

MailMessage& MailMessage::setBody(ReadWriteStream* stream, MimeType mime /* = MIME_TEXT */)
{
	if (_bodyStream) {
		debug_e("MailMessage::setBody: Discarding already set stream!");
		delete _bodyStream;
	}

	_bodyStream = stream;
	_bodyMime = mime;

	return *this;
}

MailMessage& MailMessage::addAttachment(ReadWriteStream* stream)
{
	if (!stream)
		return *this;

	String name = stream->name();
	String mime = ContentType::fromFileName(name);

	return addAttachment(stream, mime, name);
}

MailMessage& MailMessage::addAttachment(ReadWriteStream* stream, MimeType mime, const String& name)
{
	return addAttachment(stream, ContentType::toString(mime), name);
}

MailMessage& MailMessage::addAttachment(ReadWriteStream* stream, const String& mime, const String& name)
{
	HttpPartResult attachment;
	attachment.stream = stream;
	attachment.headers = new HttpHeaders();
	(*attachment.headers)[hhfn_ContentType] = mime;
	(*attachment.headers)[hhfn_ContentDisposition] = F("attachment");
	if (name)
		(*attachment.headers)[hhfn_ContentDisposition] += F("; filename=\"") + name + "\"";

	_attachments.addElement(attachment);

	return *this;
}



HttpHeaders& MailMessage::prepareHeaders()
{
	headers[hhfn_From] = from;
	headers[hhfn_To] = to;

	if (cc)
		headers[hhfn_Cc] = cc;

	headers[hhfn_Subject] = subject;
	headers[hhfn_ContentType] = ContentType::toString(_bodyMime);

	String contentTransferEncoding = F("quoted-printable");

	String contentType = ContentType::toString(_bodyMime);
	headers[hhfn_ContentType] = contentType;

	if (_attachments.count()) {
		MultipartStream* mStream = new MultipartStream(HttpPartProducerDelegate(&MailMessage::multipartProducer, this));
		HttpPartResult text;
		text.headers = new HttpHeaders();
		(*text.headers)[hhfn_ContentType] = contentType;
		(*text.headers)[hhfn_ContentTransferEncoding] = contentTransferEncoding;
		text.stream = _bodyStream;

		_attachments.insertElementAt(text, 0);

		contentTransferEncoding = nullptr;
		headers[hhfn_ContentType] = F("multipart/mixed; boundary=") + mStream->getBoundary();
		_bodyStream = mStream;
	}

	if (contentTransferEncoding)
		headers[hhfn_ContentTransferEncoding] = contentTransferEncoding;

	return headers;
}


HttpPartResult MailMessage::multipartProducer()
{
	HttpPartResult result;

	if (_attachments.count()) {
		result = _attachments[0];
		_attachments.remove(0);

		if (!result.headers->contains(hhfn_ContentTransferEncoding)) {
			result.stream = new Base64OutputStream(result.stream);
			(*result.headers)[hhfn_ContentTransferEncoding] = F("base64");
		}
	}

	return result;
}



IDataSourceStream* MailMessage::getBodyStream()
{
	if (!_bodyStream)
		return nullptr;

	auto stream = new QuotedPrintableOutputStream(_bodyStream);
	if (stream)
		_bodyStream = nullptr;

	return stream;
}

