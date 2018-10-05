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

MailMessage& MailMessage::setHeader(const String& name, const String& value)
{
	headers[name] = value;

	return *this;
}

HttpHeaders& MailMessage::getHeaders()
{
	if(!headers.contains(hhfn_From)) {
		headers[hhfn_From] = from;
	}
	if(!headers.contains(hhfn_To)) {
		headers[hhfn_To] = to;
	}
	if(!headers.contains(hhfn_Cc) && cc.length()) {
		headers[hhfn_Cc] = cc;
	}
	headers[hhfn_Subject] = subject;

	return headers;
}

MailMessage& MailMessage::setBody(const String& body, MimeType mime /* = MIME_TEXT */)
{
	MemoryDataStream* memory = new MemoryDataStream();
	int written = memory->write((uint8_t*)body.c_str(), body.length());
	if(written < body.length()) {
		debug_e("MailMessage::setBody: Unable to store the complete body");
	}

	return setBody(memory, mime);
}

MailMessage& MailMessage::setBody(ReadWriteStream* stream, MimeType mime /* = MIME_TEXT */)
{
	if(this->stream != nullptr) {
		debug_e("MailMessage::setBody: Discarding already set stream!");
		delete this->stream;
		this->stream = nullptr;
	}

	this->stream = stream;
	headers[hhfn_ContentType] = ContentType::toString(mime);

	return *this;
}

MailMessage& MailMessage::addAttachment(FileStream* stream)
{
	if(stream == nullptr) {
		return *this;
	}

	String filename = stream->fileName();
	String mime = ContentType::fromFullFileName(filename);

	return addAttachment(stream, mime, filename);
}

MailMessage& MailMessage::addAttachment(ReadWriteStream* stream, MimeType mime, const String& filename /* = "" */)
{
	return addAttachment(stream, ContentType::toString(mime), filename);
}

MailMessage& MailMessage::addAttachment(ReadWriteStream* stream, const String& mime, const String& filename /* = "" */)
{
	HttpPartResult attachment;
	attachment.stream = stream;
	attachment.headers = new HttpHeaders();
	(*attachment.headers)[hhfn_ContentType] = mime;
	(*attachment.headers)[hhfn_ContentDisposition] = F("attachment");
	if(filename.length()) {
		(*attachment.headers)[hhfn_ContentDisposition] += F("; filename=\"") + filename + '"';
	}

	attachments.addElement(attachment);

	return *this;
}
