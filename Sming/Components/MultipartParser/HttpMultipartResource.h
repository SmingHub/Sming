/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpMultipartResource.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Http/HttpServerConnection.h>
#include <Network/Http/HttpResource.h>
#include <WString.h>

typedef Delegate<void(HttpFiles&)> HttpFilesMapper;

class HttpMultipartResource : public HttpResource
{
public:
	/**
	 * @brief HttpResource that allows handling of HTTP file upload.
	 * @param mapper callback that provides information where the desired upload fields will be stored.
	 * @param complete callback that will be called after the request has completed.
	 * @note:
	 * 		 On a normal computer the file uploads are usually using
	 * 		 temporary space on the hard disk or in memory to store the incoming data.
	 * 		 On an embedded device that is a luxury that we can hardly afford.
	 * 		 Therefore we should define a `map` that specifies explicitly
	 * 		 where every form field will be stored.
	 * 		 If a field is not specified then its content will be discarded.
	 */
	HttpMultipartResource(const HttpFilesMapper& mapper, HttpResourceDelegate complete)
	{
		onHeadersComplete = HttpResourceDelegate(&HttpMultipartResource::setFileMap, this);
		onRequestComplete = complete;
		this->mapper = mapper;
	}

	virtual int setFileMap(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response);

	void shutdown(HttpServerConnection& connection) override;

private:
	HttpFilesMapper mapper;
};
