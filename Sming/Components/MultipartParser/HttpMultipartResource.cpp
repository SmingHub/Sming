/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpMultipartResource.cpp
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpMultipartResource.h"

int HttpMultipartResource::setFileMap(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	String contentType = request.headers[HTTP_HEADER_CONTENT_TYPE];
	String mimeType = ContentType::toString(MIME_FORM_MULTIPART);
	if(!(request.method == HTTP_POST && contentType.startsWith(mimeType))) {
		return 0;
	}

	mapper(request.files);

	return 0;
}

void HttpMultipartResource::shutdown(HttpServerConnection& connection)
{
}
