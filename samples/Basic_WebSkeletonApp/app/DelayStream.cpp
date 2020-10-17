#include "DelayStream.h"

void DelayStream::sendFile()
{
	auto response = connection.getResponse();

	String fn = filename + _F(".gz");
	if(::fileExist(fn)) {
		response->headers[HTTP_HEADER_CONTENT_ENCODING] = F("gzip");
	} else {
		fn = filename;
	}

	if(FileStream::open(fn, eFO_ReadOnly)) {
		debug_i("opened '%s', %u bytes", fn.c_str(), FileStream::available());
		response->setContentType(ContentType::fromFullFileName(filename));
	} else {
		debug_e("open '%s' failed!", fn.c_str());
		response->code = HTTP_STATUS_NOT_FOUND;
	}

	ready = true;
	connection.send();
}
