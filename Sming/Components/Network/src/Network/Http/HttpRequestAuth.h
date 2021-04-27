/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpRequestAuth.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "HttpResponse.h"

class HttpRequest;

class AuthAdapter
{
public:
	virtual ~AuthAdapter()
	{
	}

	virtual void setRequest(HttpRequest* request) = 0;

	virtual void setResponse(HttpResponse* response)
	{
	}
};

class HttpBasicAuth : public AuthAdapter
{
public:
	HttpBasicAuth(const String& username, const String& password) : username(username), password(password)
	{
	}

	void setRequest(HttpRequest* request) override;

private:
	String username;
	String password;
};

class HttpDigestAuth : public AuthAdapter
{
public:
	HttpDigestAuth(const String& username, const String& password) : username(username), password(password)
	{
	}

	void setRequest(HttpRequest* request) override
	{
		this->request = request;
	}

	void setResponse(HttpResponse* response) override;

private:
	String username;
	String password;
	HttpRequest* request = nullptr;
};
